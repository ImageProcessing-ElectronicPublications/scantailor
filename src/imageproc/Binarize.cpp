/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2008  Joseph Artsimovich <joseph_a@mail.ru>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <QImage>
#include <QRect>
#include <QDebug>
#include "Binarize.h"
#include "BinaryImage.h"
#include "BinaryThreshold.h"
#include "Grayscale.h"
#include "IntegralImage.h"

namespace imageproc
{

BinaryImage binarizeOtsu(
    QImage const& src,
    int const delta)
{
    BinaryThreshold const threshold(BinaryThreshold::otsuThreshold(src));
    BinaryThreshold const adjust(BinaryThreshold::adjustThreshold(threshold, delta));

    return BinaryImage(src, adjust);
}

BinaryImage binarizeMokji(
    QImage const& src,
    unsigned const max_edge_width,
    unsigned const min_edge_magnitude,
    int const delta)
{
    BinaryThreshold const threshold(
        BinaryThreshold::mokjiThreshold(
            src,
            max_edge_width,
            min_edge_magnitude
        )
    );
    BinaryThreshold const adjust(BinaryThreshold::adjustThreshold(threshold, delta));

    return BinaryImage(src, adjust);
}

BinaryImage binarizeSauvola(
    QImage const& src,
    QSize const window_size,
    double const coef,
    int const delta,
    unsigned char const lower_bound,
    unsigned char const upper_bound)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeSauvola: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    QImage const gray(toGrayscale(src));
    int const w = gray.width();
    int const h = gray.height();

    IntegralImage<uint32_t> integral_image(w, h);
    IntegralImage<uint64_t> integral_sqimage(w, h);

    uint8_t const* gray_line = gray.bits();
    int const gray_bpl = gray.bytesPerLine();

    for (int y = 0; y < h; ++y, gray_line += gray_bpl)
    {
        integral_image.beginRow();
        integral_sqimage.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = gray_line[x];
            integral_image.push(pixel);
            integral_sqimage.push(pixel * pixel);
        }
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    BinaryImage bw_img(w, h);
    uint32_t* bw_line = bw_img.data();
    int const bw_wpl = bw_img.wordsPerLine();

    double const range = 128.0;
    double const adj_d = (double) delta / range;

    gray_line = gray.bits();
    for (int y = 0; y < h; ++y)
    {
        int const top = std::max(0, y - window_lower_half);
        int const bottom = std::min(h, y + window_upper_half); // exclusive

        for (int x = 0; x < w; ++x)
        {
            int const left = std::max(0, x - window_left_half);
            int const right = std::min(w, x + window_right_half); // exclusive
            int const area = (bottom - top) * (right - left);
            assert(area > 0); // because window_size > 0 and w > 0 and h > 0

            QRect const rect(left, top, right - left, bottom - top);
            double const window_sum = integral_image.sum(rect);
            double const window_sqsum = integral_sqimage.sum(rect);

            double const r_area = 1.0 / area;
            double const mean = window_sum * r_area;
            double const sqmean = window_sqsum * r_area;

            double const variance = sqmean - mean * mean;
            double const deviation = sqrt(fabs(variance));

            double const k = coef;
            double const threshold = mean * (1.0 - k * (1.0 - deviation / range - adj_d));

            uint32_t const msb = uint32_t(1) << 31;
            uint32_t const mask = msb >> (x & 31);
            if (gray_line[x] < lower_bound ||
                    (gray_line[x] <= upper_bound &&
                     int(gray_line[x]) < threshold))
            {
                // black
                bw_line[x >> 5] |= mask;
            }
            else
            {
                // white
                bw_line[x >> 5] &= ~mask;
            }
        }

        gray_line += gray_bpl;
        bw_line += bw_wpl;
    }

    return bw_img;
}

BinaryImage binarizeWolf(
    QImage const& src,
    QSize const window_size,
    double const coef,
    int const delta,
    unsigned char const lower_bound,
    unsigned char const upper_bound)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeWolf: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    QImage const gray(toGrayscale(src));
    int const w = gray.width();
    int const h = gray.height();

    IntegralImage<uint32_t> integral_image(w, h);
    IntegralImage<uint64_t> integral_sqimage(w, h);

    uint8_t const* gray_line = gray.bits();
    int const gray_bpl = gray.bytesPerLine();

    uint32_t min_gray_level = 255;

    for (int y = 0; y < h; ++y, gray_line += gray_bpl)
    {
        integral_image.beginRow();
        integral_sqimage.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = gray_line[x];
            integral_image.push(pixel);
            integral_sqimage.push(pixel * pixel);
            min_gray_level = std::min(min_gray_level, pixel);
        }
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    std::vector<float> means(w * h, 0);
    std::vector<float> deviations(w * h, 0);

    double const range = 128.0;
    double const adj_d = (double) delta / range;
    double max_deviation = 0;

    for (int y = 0; y < h; ++y)
    {
        int const top = std::max(0, y - window_lower_half);
        int const bottom = std::min(h, y + window_upper_half); // exclusive

        for (int x = 0; x < w; ++x)
        {
            int const left = std::max(0, x - window_left_half);
            int const right = std::min(w, x + window_right_half); // exclusive
            int const area = (bottom - top) * (right - left);
            assert(area > 0); // because window_size > 0 and w > 0 and h > 0

            QRect const rect(left, top, right - left, bottom - top);
            double const window_sum = integral_image.sum(rect);
            double const window_sqsum = integral_sqimage.sum(rect);

            double const r_area = 1.0 / area;
            double const mean = window_sum * r_area;
            double const sqmean = window_sqsum * r_area;

            double const variance = sqmean - mean * mean;
            double const deviation = sqrt(fabs(variance));
            max_deviation = std::max(max_deviation, deviation);
            means[w * y + x] = mean;
            deviations[w * y + x] = deviation;
        }
    }

    // TODO: integral images can be disposed at this point.

    BinaryImage bw_img(w, h);
    uint32_t* bw_line = bw_img.data();
    int const bw_wpl = bw_img.wordsPerLine();

    uint32_t const msb = uint32_t(1) << 31;
    gray_line = gray.bits();
    for (int y = 0; y < h; ++y, gray_line += gray_bpl, bw_line += bw_wpl)
    {
        for (int x = 0; x < w; ++x)
        {
            float const mean = means[y * w + x];
            float const deviation = deviations[y * w + x];
            double const k = coef;
            double const a = 1.0 - deviation / max_deviation - adj_d;
            double const threshold = mean - k * a * (mean - min_gray_level);

            uint32_t const mask = msb >> (x & 31);
            if (gray_line[x] < lower_bound ||
                    (gray_line[x] <= upper_bound &&
                     int(gray_line[x]) < threshold))
            {
                // black
                bw_line[x >> 5] |= mask;
            }
            else
            {
                // white
                bw_line[x >> 5] &= ~mask;
            }
        }
    }

    return bw_img;
}

/*
 * window = mean * (1 - k * md / kd), k = 1.0
 * where:
 * md = (mean + 1) / (meanFull + deviation + 1)
 * kd = 1 + kdm * kds
 * kdm = (2 * meanFull + 1) / (deviation + 1)
 * deviationD = deviationMax - deviationMin
 * kds = (deviation - deviationMin) / deviationD if deviationD > 0, 1 if other
 * modification by zvezdochiot:
 * md = (mean + 1 - delta) / (meanFull + deviation + 1), delta = 0
 */
BinaryImage binarizeWindow(
    QImage const& src,
    QSize const window_size,
    double const coef,
    int const delta,
    unsigned char const lower_bound,
    unsigned char const upper_bound)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeWindow: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    QImage const gray(toGrayscale(src));
    if (gray.isNull())
    {
        return BinaryImage();
    }
    int const w = gray.width();
    int const h = gray.height();

    IntegralImage<uint32_t> integral_image(w, h);
    IntegralImage<uint64_t> integral_sqimage(w, h);

    uint8_t const* gray_line = gray.bits();
    int const gray_bpl = gray.bytesPerLine();

    for (int y = 0; y < h; ++y, gray_line += gray_bpl)
    {
        integral_image.beginRow();
        integral_sqimage.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = gray_line[x];
            integral_image.push(pixel);
            integral_sqimage.push(pixel * pixel);
        }
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    int const mean_full = BinaryThreshold::otsuThreshold(src);

    double deviation_max = 0.0;
    double deviation_min = 256.0;

    gray_line = gray.bits();
    for (int y = 0; y < h; y++)
    {
        int const top = (y > window_lower_half) ? (y - window_lower_half) : 0;
        int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h;
        for (int x = 0; x < w; x++)
        {
            int const left = (x > window_left_half) ? (x - window_left_half) : 0;
            int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w;
            int const area = (bottom - top) * (right - left);
            assert(area > 0);  // because windowSize > 0 and w > 0 and h > 0
            QRect const rect(left, top, right - left, bottom - top);
            double const window_sum = integral_image.sum(rect);
            double const window_sq_sum = integral_sqimage.sum(rect);

            double const r_area = 1.0 / area;
            double const mean = window_sum * r_area;
            double const sqmean = window_sq_sum * r_area;

            double const variance = std::fabs(sqmean - mean * mean);
            double const deviation = std::sqrt(variance);

            deviation_max = (deviation > deviation_max) ? deviation : deviation_max;
            deviation_min = (deviation < deviation_min) ? deviation : deviation_min;
        }
        gray_line += gray_bpl;
    }

    double const deviation_d = deviation_max - deviation_min;

    BinaryImage bw_img(w, h);
    uint32_t* bw_line = bw_img.data();
    int const bw_wpl = bw_img.wordsPerLine();

    uint32_t const msb = uint32_t(1) << 31;
    gray_line = gray.bits();
    for (int y = 0; y < h; y++)
    {
        int const top = (y > window_lower_half) ? (y - window_lower_half) : 0;
        int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h;
        for (int x = 0; x < w; x++)
        {
            int const left = (x > window_left_half) ? (x - window_left_half) : 0;
            int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w;
            int const area = (bottom - top) * (right - left);
            assert(area > 0);  // because windowSize > 0 and w > 0 and h > 0
            QRect const rect(left, top, right - left, bottom - top);
            double const window_sum = integral_image.sum(rect);
            double const window_sq_sum = integral_sqimage.sum(rect);

            double const r_area = 1.0 / area;
            double const mean = window_sum * r_area;
            double const sqmean = window_sq_sum * r_area;

            double const variance = std::fabs(sqmean - mean * mean);
            double const deviation = std::sqrt(variance);

            double const md = (mean + 1.0 - delta) / (mean_full + deviation + 1.0);
            double const kdm = (mean_full + mean_full + 1.0) / (deviation + 1.0);
            double const kds = (deviation_d > 0.0) ? ((deviation - deviation_min) / deviation_d) : 1.0;
            double const kd = 1.0 + kdm * kds;

            double const threshold = mean * (1.0 - coef * md / kd);

            uint32_t const mask = msb >> (x & 31);
            if (gray_line[x] < lower_bound ||
                    (gray_line[x] <= upper_bound &&
                     int(gray_line[x]) < threshold))
            {
                // black
                bw_line[x >> 5] |= mask;
            }
            else
            {
                // white
                bw_line[x >> 5] &= ~mask;
            }
        }
        gray_line += gray_bpl;
        bw_line += bw_wpl;
    }

    return bw_img;
}  // binarizeWindow

/*
 * grad = mean * k + meanG * (1.0 - k), meanG = mean(I * G) / mean(G), G = |I - mean|, k = 0.75
 * modification by zvezdochiot:
 * mean = mean + delta, delta = 0
 */
BinaryImage binarizeGrad(
    QImage const& src,
    QSize const window_size,
    double const coef,
    int const delta,
    unsigned char const lower_bound,
    unsigned char const upper_bound)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeGrad: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    QImage const gray(toGrayscale(src));
    if (gray.isNull())
    {
        return BinaryImage();
    }
    QImage gmean(toGrayscale(src));
    if (gmean.isNull())
    {
        return BinaryImage();
    }
    int const w = gray.width();
    int const h = gray.height();

    uint8_t const* gray_line = gray.bits();
    int const gray_bpl = gray.bytesPerLine();

    uint8_t* gmean_line = gmean.bits();
    int const gmean_bpl = gmean.bytesPerLine();

    IntegralImage<uint32_t> integral_image(w, h);

    for (int y = 0; y < h; ++y, gray_line += gray_bpl)
    {
        integral_image.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = gray_line[x];
            integral_image.push(pixel);
        }
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    gmean_line = gmean.bits();
    for (int y = 0; y < h; y++)
    {
        int const top = (y > window_lower_half) ? (y - window_lower_half) : 0;
        int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h;
        for (int x = 0; x < w; x++)
        {
            int const left = (x > window_left_half) ? (x - window_left_half) : 0;
            int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w;
            int const area = (bottom - top) * (right - left);
            assert(area > 0);  // because windowSize > 0 and w > 0 and h > 0
            QRect const rect(left, top, right - left, bottom - top);
            double const window_sum = integral_image.sum(rect);

            double const r_area = 1.0 / area;
            double const mean = window_sum * r_area + 0.5 + delta;
            int const imean = (int) ((mean < 0.0) ? 0.0 : (mean < 255.0) ? mean : 255.0);
            gmean_line[x] = imean;
        }
        gmean_line += gmean_bpl;
    }

    double gvalue = 127.5;
    double sum_g = 0.0, sum_gi = 0.0;
    gray_line = gray.bits();
    gmean_line = gmean.bits();
    for (int y = 0; y < h; y++)
    {
      double sum_gl = 0.0;
      double sum_gil = 0.0;
        for (int x = 0; x < w; x++)
        {
            double gi = gray_line[x];
            double g = gmean_line[x];
            g -= gi;
            g = (g < 0.0) ? -g : g;
            gi *= g;
            sum_gl += g;
            sum_gil += gi;
        }
        sum_g += sum_gl;
        sum_gi += sum_gil;
        gray_line += gray_bpl;
        gmean_line += gmean_bpl;
    }
    gvalue = (sum_g > 0.0) ? (sum_gi / sum_g) : gvalue;

    double const mean_grad = gvalue * (1.0 - coef);

    BinaryImage bw_img(w, h);
    uint32_t* bw_line = bw_img.data();
    int const bw_wpl = bw_img.wordsPerLine();

    uint32_t const msb = uint32_t(1) << 31;
    gray_line = gray.bits();
    gmean_line = gmean.bits();
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            double const origin = gray_line[x];
            double const mean = gmean_line[x];
            double const threshold = mean_grad + mean * coef;


            uint32_t const mask = msb >> (x & 31);
            if (gray_line[x] < lower_bound ||
                    (gray_line[x] <= upper_bound &&
                     int(gray_line[x]) < threshold))
            {
                // black
                bw_line[x >> 5] |= mask;
            }
            else
            {
                // white
                bw_line[x >> 5] &= ~mask;
            }
        }
        gray_line += gray_bpl;
        gmean_line += gmean_bpl;
        bw_line += bw_wpl;
    }

    return bw_img;
}  // binarizeGrad

/*
 * edgediv == EdgeDiv image prefilter before the Otsu threshold
 */
BinaryImage binarizeEdgeDiv(
    QImage const& src,
    QSize const window_size,
    double const coef_ep,
    double const coef_bd,
    int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeEdgeDiv: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    QImage gray(toGrayscale(src));
    if (gray.isNull())
    {
        return BinaryImage();
    }
    int const w = gray.width();
    int const h = gray.height();

    uint8_t* gray_line = gray.bits();
    int const gray_bpl = gray.bytesPerLine();

    IntegralImage<uint32_t> integral_image(w, h);

    for (int y = 0; y < h; ++y, gray_line += gray_bpl)
    {
        integral_image.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = gray_line[x];
            integral_image.push(pixel);
        }
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    gray_line = gray.bits();
    for (int y = 0; y < h; y++)
    {
        int const top = (y > window_lower_half) ? (y - window_lower_half) : 0;
        int const bottom = ((y + window_upper_half) < h) ? (y + window_upper_half) : h;
        for (int x = 0; x < w; x++)
        {
            int const left = (x > window_left_half) ? (x - window_left_half) : 0;
            int const right = ((x + window_right_half) < w) ? (x + window_right_half) : w;
            int const area = (bottom - top) * (right - left);
            assert(area > 0);  // because windowSize > 0 and w > 0 and h > 0
            QRect const rect(left, top, right - left, bottom - top);
            double const window_sum = integral_image.sum(rect);

            double const r_area = 1.0 / area;
            double const mean = window_sum * r_area;
            double const origin = gray_line[x];
            double retval = origin;
            if (coef_ep > 0.0)
            {
                // EdgePlus
                // edge = I / blur (shift = -0.5) {0.0 .. >1.0}, mean value = 0.5
                double const edge = (1.0 + retval) / (1.0 + mean) - 0.5;
                // edgeplus = I * edge, mean value = 0.5 * mean(I)
                double const edgeplus = origin * edge;
                // return k * edgeplus + (1 - k) * I
                retval = coef_ep * edgeplus + (1.0 - coef_ep) * origin;
            }
            if (coef_bd > 0.0)
            {
                // BlurDiv
                // edge = blur / I (shift = -0.5) {0.0 .. >1.0}, mean value = 0.5
                double const edgeinv = (1.0 + mean) / (1.0 + retval) - 0.5;
                // edgenorm = edge * k + max * (1 - k), mean value = {0.5 .. 1.0} * mean(I)
                double const edgenorm = coef_bd * edgeinv + (1.0 - coef_bd);
                // return I / edgenorm
                retval = (edgenorm > 0.0) ? (origin / edgenorm) : origin;
            }
            // trim value {0..255}
            retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
            gray_line[x] = (int) retval;
        }
        gray_line += gray_bpl;
    }

    return binarizeOtsu(gray, delta);
}  // binarizeBlurDiv

} // namespace imageproc
