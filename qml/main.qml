import QtQuick 2.11
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.4

ApplicationWindow {
  // ------------------------- Properties -------------------------

  property int buttonWidth: 100;
  property int separatorWidth: 13
  property bool isRunning: false

  // ------------------------- Functions -------------------------

  function open() {
  }

  function start() {
    isRunning = !isRunning;
    controller.startScoreFollowing();
  }

  function stop() {
    isRunning = !isRunning;
    controller.stopScoreFollowing();
  }

  function exit() {
    Qt.quit();
  }

  // ------------------------- Layout -------------------------

  title: "Score follower"
  visible: true
  width: 1024
  height: 768

  header: ToolBar {
    Row {
      anchors.fill: parent

      Button {
        id: openButton
        width: buttonWidth;
        text: "Open";
        onClicked: open();
      }
      ToolSeparator { width: separatorWidth; anchors.verticalCenter: parent.verticalCenter; }
      Button {
        id: startButton
        width: buttonWidth;
        text: "Start";
        visible: !isRunning;
        onClicked: start();
      }
      Button {
        id: stopButton;
        width: buttonWidth;
        text: "Stop";
        visible: isRunning;
        onClicked: stop();
      }
      ToolSeparator { width: separatorWidth; anchors.verticalCenter: parent.verticalCenter; }
      Rectangle {
        height: 1;
        color: "transparent";
        width: parent.width - (3 * buttonWidth) - (3 * separatorWidth);
      }
      ToolSeparator { width: separatorWidth; anchors.verticalCenter: parent.verticalCenter; }
      Button {
        id: exitButton;
        width: buttonWidth;
        text: "Exit";
        onClicked: exit();
      }
    }
  }
  Score {
    anchors.fill: parent;
  }
}
