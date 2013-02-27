import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import Yowsup 1.0

Rectangle {
    id: root
    width: units.gu(60)
    height: units.gu(80)
    color: "lightgray"
 
    property real margins: units.gu(2)
    property real buttonWidth: units.gu(9)
 
    Button {
        id: login_btn
        text: "Log in"
        onClicked: yowsup.login("somename", "somepassword");
        anchors { left: parent.left }
    }
    Error {
        text: "Some other error"
    }

    Yowsup {
        id: yowsup
        onAuthSuccess: {
                console.log("auth success for " + username);
                login_btn.text = i18n.tr("Log out");
        }
        onAuthFail: {
            //PopupUtils.open(errorPopover, yowsup);
            console.log("auth fail for " + username + ", reason: " + reason);
        }
    }
}
