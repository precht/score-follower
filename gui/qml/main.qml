// Author:  Jakub Precht

import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3

ApplicationWindow {
    // ------------------------- Properties -------------------------

    property int windowInitialWidth: 1024;
    property int windowInitialHeight: 768;
    property int buttonWidth: 100;
    property int separatorWidth: 13
    property int comboBoxWidth: 319
    property real level: 0
    property bool isLoading: false;

    // ------------------------- Functions -------------------------

    function open() {
        //  fileDialog.visible = true;

        controller.playedNotes = 0;
        isLoading = controller.openScore();
    }

    function start() {
        controller.follow = true;
    }

    function stop() {
        controller.follow = false;
    }

    function exit() {
        Qt.quit();
    }

    Connections {
        target: controller;
        onUpdateScore: {
            startButton.enabled = true;
            isLoading = false;
        }
    }

    onWidthChanged: update();
    onHeightChanged: update();

    // ------------------------- Layout -------------------------

    id: window;
    title: "Score follower";
    visible: true;
    width: windowInitialWidth;
    height: windowInitialHeight;
    color: "white";

    header: ToolBar {
        id: toolbar;
        //    height: 50;
        Row {
            //      height: 40;
            anchors.verticalCenter: parent.verticalCenter;
            anchors.right: parent.right;

            Button {
                id: openButton
                width: buttonWidth;
                text: "Open";
                enabled: !controller.follow;
                onClicked: open();
                onReleased: focus = false;
                onHoveredChanged: {
                    if (focus && !hovered)
                        focus = false;
                }
            }
            Separator { height: parent.height; width: separatorWidth; }
            Button {
                id: startButton
                width: buttonWidth;
                text: "Start";
                visible: !controller.follow;
                enabled: false;
                onClicked: start();
                onReleased: focus = false;
                onHoveredChanged: {
                    if (focus && !hovered)
                        focus = false;
                }
            }
            Button {
                id: stopButton;
                width: buttonWidth;
                text: "Stop";
                visible: controller.follow;
                onClicked: stop();
                onReleased: focus = false;
                onHoveredChanged: {
                    if (focus && !hovered)
                        focus = false;
                }
            }
            Separator { height: parent.height; width: separatorWidth; }
            Item {
                width: 20;
                height: parent.height;

                Rectangle {
                    width: 14;
                    height: 14;
                    color: (controller.follow ? "red" : "grey");
                    anchors.centerIn: parent;
                    radius: 999;
                }
            }
            Separator { height: parent.height; width: separatorWidth; }
            Item {
                height: parent.height;
                width: comboBoxWidth;

                Rectangle {
                    height: 12;
                    width: parent.width - 10; // -10 because first button is cut on high resolution display
                    anchors.centerIn: parent;
                    color: "transparent";
                    border.color: "grey";
                    border.width: 1;

                    Rectangle {
                        height: parent.height-2;
                        width: Math.min(controller.level * parent.width, parent.width - 2);
                        //            color: "#1abc9c";
                        color: "forestgreen"
                        anchors.verticalCenter: parent.verticalCenter;
                        anchors.left: parent.left;
                        anchors.leftMargin: 1;
                    }
                }
            }
            Separator { height: parent.height; width: separatorWidth; }
            Rectangle {
                height: 1;
                width: comboBoxWidth - 10;
                color: "transparent";
            }
            Separator { height: parent.height; width: separatorWidth; }
            Button {
                id: exitButton;
                width: buttonWidth;
                text: "Exit";
                onClicked: exit();
                onHoveredChanged: {
                    if (focus && !hovered)
                        focus = false;
                }
            }
            Item {
                height: parent.height;
                width: Math.max(0, (window.width - windowInitialWidth) / 2);
            }
        }
    }

    Score {
        id: score;
        anchors.fill: parent;
        visible: !isLoading;
    }

    BusyIndicator {
        id: busyIndicator;
        visible: isLoading;
        width: 100;
        height: 100;
        anchors.centerIn: parent;
    }
}
