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

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include "BlackWhiteOptions.h"

namespace output
{

BlackWhiteOptions::BlackWhiteOptions()
    : m_thresholdMethod(T_OTSU)
    , m_thresholdAdjustment(0)
    , m_thresholdManual(false)
    , m_thresholdRadius(getThresholdDefaultRadius())
    , m_thresholdCoef(getThresholdDefaultCoef())
{
}

BlackWhiteOptions::BlackWhiteOptions(QDomElement const& el)
    : m_thresholdMethod(parseThresholdMethod(el.attribute("thresholdMethod")))
    , m_thresholdAdjustment(el.attribute("thresholdAdj").toInt())
    , m_thresholdManual(el.attribute("manual") == "1")
    , m_thresholdRadius(el.attribute("thresholdRadius").toInt())
    , m_thresholdCoef(el.attribute("thresholdCoef").toDouble())
{
    if (!m_thresholdManual)
    {
        m_thresholdRadius = getThresholdDefaultRadius();
        m_thresholdCoef = getThresholdDefaultCoef();
    }
}

QDomElement
BlackWhiteOptions::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("thresholdMethod", formatThresholdMethod(m_thresholdMethod));
    el.setAttribute("thresholdAdj", m_thresholdAdjustment);
    if (m_thresholdManual)
    {
        el.setAttribute("manual", "1");
        el.setAttribute("thresholdRadius", m_thresholdRadius);
        el.setAttribute("thresholdCoef", m_thresholdCoef);
    }
    return el;
}

int
BlackWhiteOptions::getThresholdDefaultRadius() const
{
    int threshold_radius = 1;
    switch (m_thresholdMethod)
    {
    case T_OTSU:
        break;
    case T_MOKJI:
        threshold_radius = 3;
        break;
    case T_SAUVOLA:
    case T_WOLF:
        threshold_radius = 100;
        break;
    case T_WINDOW:
        threshold_radius = 50;
        break;
    case T_GRAD:
        threshold_radius = 15;
        break;
    case T_EDGEDIV:
        threshold_radius = 5;
        break;
    }

    return threshold_radius;
}

double
BlackWhiteOptions::getThresholdDefaultCoef() const
{
    double threshold_coef = 0.01;
    switch (m_thresholdMethod)
    {
    case T_OTSU:
        break;
    case T_MOKJI:
        threshold_coef = 0.08;
        break;
    case T_SAUVOLA:
        threshold_coef = 0.34;
        break;
    case T_WOLF:
        threshold_coef = 0.30;
        break;
    case T_WINDOW:
        threshold_coef = 1.00;
        break;
    case T_GRAD:
    case T_EDGEDIV:
        threshold_coef = 0.75;
        break;
    }

    return threshold_coef;
}

bool
BlackWhiteOptions::operator==(BlackWhiteOptions const& other) const
{
    if (m_thresholdMethod != other.m_thresholdMethod)
    {
        return false;
    }
    if (m_thresholdAdjustment != other.m_thresholdAdjustment)
    {
        return false;
    }
    if (m_thresholdManual != other.m_thresholdManual)
    {
        return false;
    }
    if (m_thresholdRadius != other.m_thresholdRadius)
    {
        return false;
    }
    if (m_thresholdCoef != other.m_thresholdCoef)
    {
        return false;
    }

    return true;
}

bool
BlackWhiteOptions::operator!=(BlackWhiteOptions const& other) const
{
    return !(*this == other);
}

ThresholdFilter
BlackWhiteOptions::parseThresholdMethod(QString const& str)
{
    if (str == "mokji")
    {
        return T_MOKJI;
    }
    else if (str == "sauvola")
    {
        return T_SAUVOLA;
    }
    else if (str == "wolf")
    {
        return T_WOLF;
    }
    else if (str == "window")
    {
        return T_WINDOW;
    }
    else if (str == "grad")
    {
        return T_GRAD;
    }
    else if (str == "edgediv")
    {
        return T_EDGEDIV;
    }
    else
    {
        return T_OTSU; /* default: "otsu" */
    }
}

QString
BlackWhiteOptions::formatThresholdMethod(ThresholdFilter type)
{
    QString str = "";
    switch (type)
    {
    case T_OTSU:
        str = "otsu";
        break;
    case T_MOKJI:
        str = "mokji";
        break;
    case T_SAUVOLA:
        str = "sauvola";
        break;
    case T_WOLF:
        str = "wolf";
        break;
    case T_WINDOW:
        str = "window";
        break;
    case T_GRAD:
        str = "grad";
        break;
    case T_EDGEDIV:
        str = "edgediv";
        break;
    }
    return str;
}

} // namespace output
