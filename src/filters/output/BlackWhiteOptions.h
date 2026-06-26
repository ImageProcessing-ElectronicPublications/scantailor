/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#ifndef OUTPUT_BLACK_WHITE_OPTIONS_H_
#define OUTPUT_BLACK_WHITE_OPTIONS_H_

class QString;
class QDomDocument;
class QDomElement;

namespace output
{

enum ThresholdFilter { T_OTSU, T_MOKJI, T_SAUVOLA, T_WOLF, T_WINDOW, T_GRAD, T_EDGEDIV };

class BlackWhiteOptions
{
public:
    BlackWhiteOptions();

    BlackWhiteOptions(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    ThresholdFilter getThresholdMethod() const
    {
        return m_thresholdMethod;
    }

    void setThresholdMethod(ThresholdFilter val)
    {
        m_thresholdMethod = val;
    }

    int getThresholdAdjustment() const
    {
        return m_thresholdAdjustment;
    }

    void setThresholdAdjustment(int val)
    {
        m_thresholdAdjustment = val;
    }

    bool getThresholdManual() const
    {
        return m_thresholdManual;
    }

    void setThresholdManual(bool val)
    {
        m_thresholdManual = val;
    }

    int getThresholdRadius() const
    {
        return m_thresholdRadius;
    }

    void setThresholdRadius(int val)
    {
        m_thresholdRadius = val;
    }

    double getThresholdCoef() const
    {
        return m_thresholdCoef;
    }

    void setThresholdCoef(double val)
    {
        m_thresholdCoef = val;
    }

    int getThresholdDefaultRadius() const;

    double getThresholdDefaultCoef() const;

    bool operator==(BlackWhiteOptions const& other) const;

    bool operator!=(BlackWhiteOptions const& other) const;
private:
    ThresholdFilter m_thresholdMethod;
    int m_thresholdAdjustment;
    bool m_thresholdManual;
    int m_thresholdRadius;
    double m_thresholdCoef;

    static ThresholdFilter parseThresholdMethod(QString const& str);

    static QString formatThresholdMethod(ThresholdFilter type);
};

} // namespace output

#endif
