/*!
 * Copyright (C) 2014 Nomovok Ltd. All rights reserved.
 * Contact: info@nomovok.com
 *
 * This file may be used under the terms of the GNU Lesser
 * General Public License version 2.1 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU Lesser General Public License version 2.1 requirements
 * will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * In addition, as a special exception, copyright holders
 * give you certain additional rights.  These rights are described in
 * the Digia Qt LGPL Exception version 1.1, included in the file
 * LGPL_EXCEPTION.txt in this package.
 */

import QtQuick 2.0
import "qml"
import "qml/content"
import Charts 1.0

Item{

    // qml js item usage
    QmlJSItems{
    }

    // c++ plugin extention use
    Item {
        x: 200
        width: 100;

        QmlSubItem {
            y: 0
            color: "red"
            Text {
                 text: "C++ Pie chart"
                 font.family: "Helvetica"
                 font.pointSize: 12
                 color: "black"
             }
        }

        QmlInPlugin{
            y: 100;
            PieChart {
                anchors.centerIn: parent
                width: 100; height: 100

                slices: [
                    PieSlice {
                        anchors.fill: parent
                        color: "red"
                        fromAngle: 0; angleSpan: 110
                    },
                    PieSlice {
                        anchors.fill: parent
                        color: "green"
                        fromAngle: 110; angleSpan: 50
                    },
                    PieSlice {
                        anchors.fill: parent
                        color: "blue"
                        fromAngle: 160; angleSpan: 100
                    }
                ]
            }
        }

    }
}
