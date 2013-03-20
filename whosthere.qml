/*
 * Copyright (C) 2013 Matthias Gehre <gehre.matthias@gmail.com>
 *
 * This work is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

import QtQuick 2.0
import QtQuick.LocalStorage 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components.Popups 0.1
import WhosThere 1.0
import "db.js" as DB
import "util.js" as Util

MainView {
    id: root
    width: units.gu(42)
    height: units.gu(67)

    Component.onCompleted: {
        Util.log("Component.onCompleted");
        pagestack.push(page_contacts);
        DB.updateContacts();
    }

    PageStack {
        id: pagestack
        anchors.fill: parent

        Page {
            id: page_loading
            visible: false
            anchors.fill: parent
            title: i18n.tr("Connecting...")
            ActivityIndicator {
                id: activity_ind
                anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter}
                running: true
            }
            Label {
                text: i18n.tr("Connecting ...")
                anchors { horizontalCenter: parent.horizontalCenter; bottom: activity_ind.top; margins: units.gu(4) }
            }
        }

        Page {
            id: page_login
            visible: false
            title: i18n.tr("Account Settings")
            anchors.fill: parent
            Column {
                anchors.fill: parent
                Label {
                    anchors.margins: units.gu(2)
                    text: i18n.tr("Account")
                    font.underline: true
                    font.italic: true
                }
                Label {
                    anchors.margins: units.gu(2)
                    text: i18n.tr("Login with your mobile number (including country code) and password. To obtain a password, see section 'Register' below.")
                    wrapMode: Text.WordWrap
                    anchors { left: parent.left; right: parent.right }
                }
                TextField {
                    anchors.margins: units.gu(2)
                    id: username_txt
                    placeholderText: i18n.tr("Telephone number including country code")
                    width: parent.width; height: units.gu(4)
                    //Starts with non-zero or + or 00
                    validator: RegExpValidator { regExp: /(\+|00|)[1-9]\d+/ }
                }
                TextField {
                    anchors.margins: units.gu(2)
                    id: password_txt
                    placeholderText: i18n.tr("Password")
                    width: parent.width; height: units.gu(4)
                }
                Row {
                    Label {
                        text: i18n.tr("Enabled: ")
                    }
                    CheckBox {
                        id: accountEnabled_chk
                        onClicked: {
                            whosthere.enableAccount( checked );
                        }
                    }
                    Label {
                        text: i18n.tr("Always connected: ")
                    }
                    CheckBox {
                        id: alwaysConnected_chk
                        onClicked: {
                            whosthere.alwaysConnected( checked );
                        }
                    }
                }
                Row {
                    Button {
                        anchors.margins: units.gu(2)
                        id: login_btn
                        text: i18n.tr("Save")
                        onClicked: {
                            if( username_txt.text == "" || password_txt.text == "")
                                return;
                            var phonenumber = sanitizePhoneNumber(username_txt.text);
                            if(!phonenumber) {
                                alert("You entered an invalid international phone number");
                                return;
                            }
                            whosthere.set_account(phonenumber, password_txt.text);
                        }
                    }
                    Button {
                        id: removeAccount_btn
                        anchors.margins: units.gu(2)
                        width: units.gu(20)
                        text: i18n.tr("Remove")
                        onClicked: {
                            whosthere.remove_account();
                        }
                    }
                }
                Label {
                    anchors.margins: units.gu(2)
                    text: i18n.tr("Register")
                    font.italic: true
                    font.underline: true
                }
                Label {
                    anchors.margins: units.gu(2)
                    text: i18n.tr("Registration is a two step process: First, enter your phone number including country code "
                                  +"above, "
                                  +"then tap 'Request code'. A code will be send via text to your phone number. "
                                  +"Enter the code below and click 'Request password'. The password will appear in "
                                  +"the password field above. Please note it down! You will automatically log in.")
                    wrapMode: Text.WordWrap
                    anchors { left: parent.left; right: parent.right }
                }
                Row {
                    Label {
                        text: "1. "
                    }
                    Button {
                        id: reqestCode_btn
                        anchors.margins: units.gu(2)
                        text: i18n.tr("Request code")
                        width: units.gu(18)
                        onClicked: {
                            var phonenumber = sanitizePhoneNumber(username_txt.text);
                            if(!phonenumber) {
                                alert("You entered an invalid international phone number");
                                return;
                            }
                            var cc = whosthere.getCountryCode(phonenumber);
                            if(!cc) {
                                alert("Could not determine your country code. Please report this as a bug.");
                                return;
                            }
                            phonenumber = phonenumber.slice(cc.length)
                            console.log("onClicked: call code_request cc:" + cc + " phonenumber:" + phonenumber);
                            whosthere.code_request(cc, phonenumber, DB.getUID(), true);
                        }
                    }
                }
                Row {
                    Label {
                        text: "2. "
                    }
                    anchors.margins: units.gu(2)
                    TextField {
                        anchors.margins: units.gu(2)
                        id: code_txt
                        placeholderText: i18n.tr("Code")
                        width: units.gu(12)
                        validator: RegExpValidator { regExp: /(\d{3}-?\d{3})|/ }
                    }
                    Button {
                        //anchors.left: code_txt.right
                        anchors.margins: units.gu(2)
                        text: i18n.tr("Request password")
                        width: units.gu(20)
                        onClicked: {
                            console.log("onClicked: call code_register");
                            var phonenumber = sanitizePhoneNumber(username_txt.text);
                            if(!phonenumber) {
                                alert("You entered an invalid international phone number");
                                return;
                            }
                            var cc = whosthere.getCountryCode(phonenumber);
                            if(!cc) {
                                alert("Could not determine your country code. Please report this as a bug.");
                                return;
                            }
                            phonenumber = phonenumber.slice(cc.length)
                            //Remove hyphon if it exists
                            var code = code_txt.text.replace('-','');
                            if(code.length != 6) {
                                alert("You entered an invalid code. It should contain 6 digits.");
                                return;
                            }
                            whosthere.code_register(cc, phonenumber, DB.getUID(), code);
                        }
                    }
                }
            }
            Label {
                anchors.bottom: parent.bottom
                anchors.margins: units.gu(2)
                text: i18n.tr("By connecting you agree to<br><a href='http://www.whatsapp.com/legal/#TOS'>Whatsapp's terms of service</a>");
                onLinkActivated: Qt.openUrlExternally(link)
                wrapMode: Text.WordWrap
            }

            tools: ToolbarActions {
                Action {
                    text: "Contacts"
                    onTriggered: {
                        pagestack.push(page_contacts);
                    }
                }
                Action {
                    text: i18n.tr("Quit")
                    onTriggered: whosthere.quit();
                }
            }
        }
        /* page_contacts */
        Page {
            id: page_contacts
            visible: false
            title: i18n.tr("Contacts")
            anchors.fill: parent
            states: [
                State {
                    name: "connected"
                    PropertyChanges {
                        target: online_status_btn
                        text: i18n.tr("connected")
                    }
                    StateChangeScript {
                        script: { Util.log("State changed to connected") }
                    }
                },
                State {
                    name: "connecting"
                    PropertyChanges {
                        target: online_status_btn
                        text: i18n.tr("connecting")
                    }
                    StateChangeScript {
                        script: { Util.log("State changed to connecting") }
                    }
                },
                State {
                    name: "disconnected"
                    PropertyChanges {
                        target: online_status_btn
                        text: i18n.tr("disconnected")
                    }
                    StateChangeScript {
                        script: { Util.log("State changed to disconnected") }
                    }
                }
            ]
            state: "disconnected"
            Button {
                id: online_status_btn
                anchors { left: parent.left; right: parent.right; top: parent.top; margins: units.gu(2) }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(page_contacts.state == "disconnected") {
                            page_contacts.state == "connecting";
                            whosthere.connectAccount();
                        } else {
                            whosthere.disconnect();
                        }
                    }
                }
            }
            Label {
                anchors { left: parent.left; right: parent.right; top: online_status_btn.bottom; margins: units.gu(4) }
                visible: contactsModel.count == 0
                text: i18n.tr("You don't have any contacts yet. Use 'Sync contacts' from the menu or receive a message from a friend!")
                wrapMode: Text.WordWrap
            }
            ListModel {
                id: contactsModel
            }
            ListView {
                anchors { left: parent.left; right: parent.right; top: online_status_btn.bottom; bottom: parent.bottom; margins: units.gu(2) }
                model: contactsModel
                delegate: ListItem.Subtitled {
                    text: displayName + (presence ? " (" + presence + ")" : "")
                          //" ( time: " + lastTime + " )"
                    subText: lastMsg ? (i18n.tr("Last message: ") + lastMsg) : ""
                    MouseArea {
                        anchors.fill: parent
                        onClicked: DB.showConversation(jid);
                    }
                }
            }
            tools: ToolbarActions {
                Action {
                    text: "Sync\ncontacts"
                    onTriggered: {
                        whosthere.syncAddressbook();
                    }
                }
                Action {
                    text: "Settings"
                    onTriggered: {
                        pagestack.push(page_login);
                    }
                }
                Action {
                    text: "About"
                    onTriggered: {
                        alert('WhosThere\n\nWritten by Matthias Gehre\n\nLogo by Seifar\n\n\nhttp://launchpad.net/whosthere');
                    }
                }
                Action {
                    text: i18n.tr("Quit")
                    onTriggered: whosthere.quit();
                }
            }
        }
        /* page_conversation */
        Page {
            property string jid
            id: page_conversation
            visible: false
            anchors.fill: parent
            title: i18n.tr("Conversation with " + DB.displayName(jid))
            ListModel {
                id: conversationMessages
            }

            Row {
                id: status_row
                Label {
                    text: i18n.tr("Status: ")
                }
                Label {
                    id: contactPresence_lbl
                    text: ""
                }
            }

            /* List view showing the messages */
            ListView {
                anchors {top: status_row.bottom; left: parent.left; right: parent.right; bottom: newMessage_inpt.top }
                anchors.margins: units.gu(2)
                model: conversationMessages
                delegate: Item {
                    anchors { left: parent.left; right: parent.right }
                    height: units.gu(8)
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if(url)
                                Qt.openUrlExternally(url)
                            else if(type == "location")
                                Qt.openUrlExternally("https://maps.google.com/maps?q="+latitude+","+longitude);
                        }
                    }
                    /* This state right aligns and flips order of contents */
                    states: State {
                             name: "incoming"
                             AnchorChanges {
                                 target: preview_img
                                 anchors.right: parent.right
                                 anchors.left: undefined
                             }
                             AnchorChanges {
                                 target: txtclm
                                 anchors.right: preview_img.left
                                 anchors.left: undefined
                             }
                             AnchorChanges {
                                 target: content_itm
                                 anchors.left: undefined
                                 anchors.right: parent.right
                             }
                    }
                    state: incoming ? "incoming" : "";
                    Item {
                        id: content_itm
                        anchors.left: parent.left
                        width: txtclm.width + preview_img.width
                        Column {
                            id: txtclm
                            anchors.left: preview_img.right
                            Label { //Text
                                text: (type == "message" ? content : type) + (name ? " " + name : "")
                            }
                            Label { //Status
                                text: (timestamp ? (Util.formatTime(timestamp) + " ") : "" )
                                      +  (sent ? "sent " : "") + (delivered ? "delivered" : "")
                                font.italic: true
                                font.pointSize: 6
                            }
                        }
                        Image {
                            id: preview_img
                            anchors.margins: units.gu(2)
                            anchors.left: parent.left
                            //Seems that the preview images are always 100x75, 75x100 or 100x100 (for location)
                            width: preview ? (sourceSize.height > sourceSize.width ? units.gu(3) : units.gu(4)) : 0
                            height: preview ? (sourceSize.height >= sourceSize.width ? units.gu(4) : units.gu(3)) : 0
                            visible: preview
                            source: preview ? "image://drawable/" + msgId : ""
                        }
                    }
                }
            }
            TextField {
                id: newMessage_inpt
                placeholderText:  i18n.tr("Type message")
                //color: "white"
                height: units.gu(4)
                anchors {bottom: parent.bottom; left: parent.left; right: parent.right }
                onAccepted: {
                    if( text === "" )
                        return;
                    whosthere.message_send(page_conversation.jid, text)
                    text = "";
                }
            }
        }
    }
    WhosThere {
        id: whosthere

        onAccountOk: {
            Util.log("onAccountOk");
            removeAccount_btn.enabled = true;
            if(pagestack.currentPage == page_login || pagestack.currentPage == page_loading)
                pagestack.push(page_contacts);
        }
        onNoAccount: {
            Util.log("onNoAccount");
            removeAccount_btn.enabled = false;
            accountEnabled_chk.checked = true;
            alwaysConnected_chk.checked = false;
            pagestack.push(page_login);
        }
        onConnectionStatusChanged: {
            Util.log("onConnectionStatusChanged status: " + status + " reason: " + reason);
            page_contacts.state = status;
            if(status == "connected" && pagestack.currentPage == page_login)
                pagestack.push(page_contacts);

            if(status == "disconnected") {
                if(reason == "authentication failed") {
                    alert("Authentication failed, check credentials");
                    pagestack.push(page_login);
                } else {
                    pagestack.push(page_contacts);
                }
            }
        }

        onAccountEnabledChanged: {
            Util.log("onAccountEnabledChanged " + enabled);
            accountEnabled_chk.checked = enabled;
            if(!enabled)
                pagestack.push(page_login);
        }
        onAccountAlwaysConnectedChanged: {
            Util.log("onAccountAlwaysConnectedChanged " + enabled);
            alwaysConnected_chk.checked = enabled;
        }

        onAccountValidityChanged: {
            Util.log("onAccountValidityChanged " + valid);
            if(!valid) {
                //TODO: remove parameters
                pagestack.push(page_login);
            }
        }

        onAccountParametersChanged: {
            Util.log("onAccountParametersChanged " + parameters);
            if(parameters["password"])
                password_txt.text = parameters["password"];
            if(parameters["account"])
                username_txt.text = parameters["account"];
        }
        onNewMessage: {
            DB.addMessage(data);
        }

        onMessageDelivered: {
            console.log("OnReceipt_messageDelivered: " + jid + " " + msgId);
            DB.setDelivered(jid,msgId);
        }
        onMessageSent: {
            console.log("OnReceipt_messageSent: " + jid + " " + msgId);
            DB.setSent(jid,msgId);
        }
        onNewContact: {
            Util.log("onNewContact "+ jid);
            if(!DB.hasContact(jid)) {
                DB.addContact(jid);
                DB.updateContacts();
            }
        }
        onPresenceChanged: {
            Util.log("onPresenceChanged: " + jid + " " + presence);
            DB.presences[jid] = presence;
            DB.updateContacts();
            if(jid == page_conversation.jid)
                contactPresence_lbl.text = presence;
        }

        /* Registration */
        onCode_request_response: {
            Util.log("onCode_request_response: " + status );
            if( status != 'sent' ) {
                alert("Reqeust failed! Status: " + status + " Reason: " + reason);
            }
        }
        onCode_register_response: {
            Util.log("onCode_register_response: " + status);
            if(status == 'ok') {
                var phonenumber = sanitizePhoneNumber(username_txt.text);
                if(!phonenumber) {
                    alert("You entered an invalid international phone number");
                    return;
                }
                set_account(phonenumber, pw);
            } else {
                alert("Registration failed! Status: " + status);
            }
        }
        onAlert: {
            root.alert(message)
        }
        /* Emitted when the QContactManager finished loading/got a new contact */
        onAddressbookReady: {
            Util.log("onAddressbookReady");
            for(var i = 0; i < contactsModel.count; ++i ) {
                var entry = contactsModel.get(i);
                entry["displayName"] = DB.displayName(entry["jid"]);
            }
        }

    }
    Component {
        id: dialogComponent
        Dialog {
            id: dialogue
            //anchors.horizontalCenter: parent.horizontalCenter
            Button {
                text: i18n.tr("Close")
                onClicked: PopupUtils.close(dialogue)
            }
        }
    }

    function getPreviewImage(id) {
        return DB.getPreviewImage(id);
    }

    function alert(message) {
        Util.log("alert: "+ message);
        //popover_lbl.text = message;
        PopupUtils.open(dialogComponent, pagestack, {"text": message})
    }

    function sanitizePhoneNumber(number) {
        if( number.indexOf("+") == 0 )
            return number.slice(1);
        if( number.indexOf("00") == 0 )
            return number.slice(2);
        if( number.indexOf("0") == 0 )
            return ""; //not an international number
        return number;
    }
}
