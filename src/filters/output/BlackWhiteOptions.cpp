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

#include "BlackWhiteOptions.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace output
{

BlackWhiteOptions::BlackWhiteOptions()
    : m_thresholdMethod(0)
    , m_thresholdAdjustment(0)
    , m_thresholdManual(false)
    , m_thresholdRadius(50)
    , m_thresholdCoef(0.3)
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
    el.setAttribute("manual", m_thresholdManual ? "1" : "0");
    el.setAttribute("thresholdRadius", m_thresholdRadius);
    el.setAttribute("thresholdCoef", m_thresholdCoef);
    return el;
}

int
BlackWhiteOptions::getThresholdDefaultRadius() const
{
    int threshold_radius = 1;
    switch (m_thresholdMethod)
    {
    case 0:
        break;
    case 1:
        threshold_radius = 3;
        break;
    case 2:
    case 3:
        threshold_radius = 100;
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
    case 0:
        break;
    case 1:
        threshold_coef = 0.08;
        break;
    case 2:
        threshold_coef = 0.34;
        break;
    case 3:
        threshold_coef = 0.30;
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

int
BlackWhiteOptions::parseThresholdMethod(QString const& str)
{
    if (str == "mokji")
    {
        return 1;
    }
    else if (str == "sauvola")
    {
        return 2;
    }
    else if (str == "wolf")
    {
        return 3;
    }
    else
    {
        return 0; /* default: "otsu" */
    }
}

QString
BlackWhiteOptions::formatThresholdMethod(int type)
{
    QString str = "";
    switch (type)
    {
    case 0:
        str = "otsu";
        break;
    case 1:
        str = "mokji";
        break;
    case 2:
        str = "sauvola";
        break;
    case 3:
        str = "wolf";
        break;
    }
    return str;
}

} // namespace output
