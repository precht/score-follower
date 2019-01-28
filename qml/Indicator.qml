import QtQuick 2.0

Rectangle {
  property int indicatorX: 0;
  property int indicatorY: 0;
  property int position: -1;

  visible: (position === ((controller.playedNotes - 1) % controller.notesPerPage + 1));
  color: "orange";
  width: controller.indicatorWidth * controller.indicatorScale;
  height: controller.indicatorHeight * 2 * controller.indicatorScale;
  x: indicatorX * controller.indicatorScale;
  y: indicatorY * controller.indicatorScale - (height - controller.indicatorHeight * controller.indicatorScale) / 2;
}
