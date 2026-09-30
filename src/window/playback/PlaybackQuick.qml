import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        color: "#162633"
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 6

        Button {
            id: playPauseButton
            enabled: playbackController.controlsEnabled
            Layout.rightMargin: 8

            icon.source: playbackController.atEnd && !playbackController.playing
                ? "qrc:/src/window/playback/icons/reset.svg"
                : (playbackController.playing ? "qrc:/src/window/playback/icons/pause.svg"
                    : "qrc:/src/window/playback/icons/play.svg")

            onClicked: playbackController.togglePlayPause()
        }

        Slider {
            id: timelineSlider
            enabled: playbackController.controlsEnabled
            Layout.fillWidth: true

            to: 1.0

            value: playbackController.sliderPosition
            onMoved: playbackController.sliderMoved(value)
        }

        Button {
            id: speedButton
            text: playbackController.speedLabel
            Layout.preferredWidth: implicitWidth

            onClicked: playbackController.openTimeStepDialog()
        }

        // Jump
        Button {
            id: jumpButton
            text: "Jump"
            enabled: playbackController.controlsEnabled

            onClicked: playbackController.openJumpDialog()
        }

        Label {
            id: timeLabel
            text: playbackController.timeLabel

            // Mirror the fixed-width font the widget pulls from QFontDatabase
            font.family: "Monospace"
        }
    }
}