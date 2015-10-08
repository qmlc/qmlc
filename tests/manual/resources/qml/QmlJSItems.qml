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
import "content/testscript1.js" as SS
import "content"

Item {

    Rectangle {
        id: rect0
        x: 0

        QmlSubItem {
            id: subitem0
            y: 0
            color: "red"
            Text {
                 text: "RGB"
                 font.family: "Helvetica"
                 font.pointSize: 12
                 color: "black"
             }
        }

        QmlSubItem {
            id: subitem1
            y: 100
            color: "green"
        }

        QmlSubItem {
            id: subitem2
            y: { return SS.testfunc(subitem1.y) }
            color: "blue"
        }
    }

    Rectangle {
        id: rect1
        x: 100
        width: 100

        height: { return SS.testfunc(width) } // height should be double the width
        color: "yellow"
        Text {
             text: "h = 2 * w";
             font.family: "Helvetica"
             font.pointSize: 12
             color: "black"
         }
    }
}
