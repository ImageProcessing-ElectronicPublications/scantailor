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
    , m_thresholdRadius(50)
    , m_thresholdCoef(0.3)
{
}

BlackWhiteOptions::BlackWhiteOptions(QDomElement const& el)
    : m_thresholdMethod(el.attribute("thresholdMethod").toInt())
    , m_thresholdAdjustment(el.attribute("thresholdAdj").toInt())
    , m_thresholdRadius(el.attribute("thresholdRadius").toInt())
    , m_thresholdCoef(el.attribute("thresholdCoef").toDouble())
{
}

QDomElement
BlackWhiteOptions::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("thresholdMethod", m_thresholdMethod);
    el.setAttribute("thresholdAdj", m_thresholdAdjustment);
    el.setAttribute("thresholdRadius", m_thresholdRadius);
    el.setAttribute("thresholdCoef", m_thresholdCoef);
    return el;
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

} // namespace output
