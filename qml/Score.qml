import QtQuick 2.0

Item {
  property int imageWidth: 874;
  property int imageHeight: 620;
  property string imageFileName: "file:///tmp/score-follower/score-page1.png";

  function updateScore() {
    scoreImage.source = "";
    scoreImage.source = imageFileName;
  }

  Connections {
    target: controller;
    onUpdateScore: updateScore();
  }

  // ------------------------- Layout -------------------------
  Image {
    id: scoreImage
    width: imageWidth;
    height: imageHeight;
    anchors.centerIn: parent;
    source: imageFileName;
  }
}
