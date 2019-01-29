import QtQuick 2.0
import QtQuick.Layouts 1.3

Item {
  property int imageWidth: 932;
  property int imageHeight: 661;

  function updateScore() {
    updatePage();
    createIndicators(controller.scoreLength);
  }

  function updatePage() {
    scoreImage.source = "";
    scoreImage.source = "file:///tmp/score-follower/score-page" + controller.currentPage + ".png";
  }

  function createIndicators(count) {
    indicators.children = "";
    var indicatorComponent = Qt.createComponent("Indicator.qml");
    for (var index = 0; index < count; index++) {
      var object = indicatorComponent.createObject(indicators);
      object.indicatorX = controller.indicatorX(index);
      object.indicatorY = controller.indicatorY(index);
      object.position = index + 1;
    }
  }

  Connections {
    target: controller;
    onUpdateScore: updateScore();
    onCurrentPageChanged: updatePage();
  }

  // ------------------------- Layout -------------------------

  RowLayout {
    anchors.fill: parent

    Image {
      id: scoreImage
      width: imageWidth;
      height: imageHeight;
      fillMode: Image.PreserveAspectFit

      Layout.preferredWidth: parent.width;
      Layout.preferredHeight: parent.height;
      Layout.maximumWidth: imageWidth;
      Layout.maximumHeight: imageHeight;
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter;

      Rectangle {
        id: indicators;
        width: scoreImage.paintedWidth;
        height: scoreImage.paintedHeight;
        color: "transparent";
//        border.color: "orange";
//        border.width: 1;

        anchors.centerIn: parent;
        onWidthChanged: {
          controller.indicatorScale = Math.min(width / imageWidth, height / imageHeight);
        }
        onHeightChanged: {
          controller.indicatorScale = Math.min(width / imageWidth, height / imageHeight);
        }
      }
    }
  }
}
