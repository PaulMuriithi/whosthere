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
        DB.loadMessages();
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
            title: i18n.tr("Login/Register")
            anchors.fill: parent
            Column {
                anchors.fill: parent
                Label {
                    anchors.margins: units.gu(1)
                    text: i18n.tr("Login")
                    font.underline: true
                    font.italic: true
                }
                Label {
                    anchors.margins: units.gu(1)
                    text: i18n.tr("Login with your mobile number (including country code, excluding leading + or 00) and password. To obtain a password, see section 'Register' below.")
                    wrapMode: Text.WordWrap
                    anchors { left: parent.left; right: parent.right }
                }
                TextField {
                    anchors.margins: units.gu(1)
                    id: username_txt
                    placeholderText: i18n.tr("Telephone number without leading + or 00")
                    width: parent.width; height: units.gu(4)
                }
                TextField {
                    anchors.margins: units.gu(1)
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
                }
                Row {
                    Button {
                        anchors.margins: units.gu(1)
                        id: login_btn
                        text: i18n.tr("Save")
                        onClicked: {
                            if( username_txt.text == "" || password_txt.text == "")
                                return;

                            whosthere.set_account(username_txt.text, password_txt.text);
                        }
                    }
                    Button {
                        id: removeAccount_btn
                        anchors.margins: units.gu(1)
                        width: units.gu(20)
                        text: i18n.tr("Remove")
                        onClicked: {
                            whosthere.remove_account();
                        }
                    }
                }
                Label {
                    anchors.margins: units.gu(1)
                    text: i18n.tr("Register")
                    font.italic: true
                    font.underline: true
                }
                Label {
                    anchors.margins: units.gu(1)
                    text: i18n.tr("Registration is a two step process: First, enter your country code (e.g. '1' for US) "
                                  +"and telephone number below (without country code and without leading 0), "
                                  +"then tap 'Request code'. A code will be send via text to your number. "
                                  +"Enter the code below and click 'Request password'. The password will appear in "
                                  +"the password field above. Please save it somewhere! You can now login.")
                    wrapMode: Text.WordWrap
                    anchors { left: parent.left; right: parent.right }
                }
                Row {
                    anchors.margins: units.gu(1)
                    TextField {
                        id: countrycode_txt
                        placeholderText: i18n.tr("cc")
                        width: units.gu(10)
                    }
                    TextField {
                        id: username_reg_txt
                        placeholderText: i18n.tr("mobile number wo. cc")
                        //height: 28
                    }
                }
                Button {
                    anchors.margins: units.gu(1)
                    text: i18n.tr("Request code")
                    width: units.gu(18)
                    onClicked: {
                        if(username_reg_txt.text == "" || countrycode_txt.text == "")
                            return;
                        console.log("onClicked: call code_request");
                        whosthere.code_request(countrycode_txt.text, username_reg_txt.text, DB.getUID(), true);
                    }
                }
                TextField {
                    anchors.margins: units.gu(1)
                    id: code_txt
                    placeholderText: i18n.tr("Code you got via text")
                    //height: 28
                }
                Button {
                    anchors.margins: units.gu(1)
                    text: i18n.tr("Request password")
                    width: units.gu(22)
                    onClicked: {
                        if(username_reg_txt.text == "" || countrycode_txt.text == ""
                                || code_txt.text == "")
                            return;
                        console.log("onClicked: call code_register");
                        //Remove hyphon if it exists
                        var code = code_txt.text.replace('-','');

                        whosthere.code_register(countrycode_txt.text, username_reg_txt.text, DB.getUID(), code);
                    }
                }

                Label {
                    anchors.margins: units.gu(1)
                    text: i18n.tr("By connecting you agree to <a href='http://www.whatsapp.com/legal/#TOS'>Whatsapp's terms of service</a>");
                    onLinkActivated: Qt.openUrlExternally(link)
                    wrapMode: Text.WordWrap
                }
            }
            tools: ToolbarActions {
                Action {
                    text: "back"
                    onTriggered: {
                        pagestack.push(page_contacts);
                    }
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
                        target: online_status_lbl
                        text: i18n.tr("connected")
                    }
                    StateChangeScript {
                        script: { Util.log("State changed to connected") }
                    }
                },
                State {
                    name: "connecting"
                    PropertyChanges {
                        target: online_status_lbl
                        text: i18n.tr("connecting")
                    }
                    StateChangeScript {
                        script: { Util.log("State changed to connecting") }
                    }
                },
                State {
                    name: "disconnected"
                    PropertyChanges {
                        target: online_status_lbl
                        text: i18n.tr("disconnected")
                    }
                    StateChangeScript {
                        script: { Util.log("State changed to disconnected") }
                    }
                }
            ]
            state: "disconnected"
            Label {
                id: online_status_lbl
                anchors { left: parent.left; right: parent.right; top: parent.top; margins: units.gu(2) }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if(page_contacts.state == "disconnected")
                            whosthere.connectAccount();
                    }
                }
            }
            Label {
                anchors { left: parent.left; right: parent.right; top: online_status_lbl.bottom; margins: units.gu(4) }
                visible: contactsModel.count == 0
                text: i18n.tr("You don't have any contacts yet. Receive a message from a friend!")
                wrapMode: Text.WordWrap
            }
            ListModel {
                id: contactsModel
            }
            ListView {
                anchors { left: parent.left; right: parent.right; top: online_status_lbl.bottom; margins: units.gu(2) }
                model: contactsModel
                delegate: ListItem.Subtitled {
                    text: jid + " ( time: " + timestamp + " )"
                    subText: i18n.tr("Last message: ") + content
                    MouseArea {
                        anchors.fill: parent
                        onClicked: DB.showConversation(jid);
                    }
                }
            }
            tools: ToolbarActions {
                Action {
                    text: "Settings"
                    onTriggered: {
                        pagestack.push(page_login);
                    }
                }
            }
        }
        /* page_conversation */
        Page {
            property string jid
            id: page_conversation
            visible: false
            anchors.fill: parent
            title: i18n.tr("Conversation with " + jid)
            Button {
                id: back_btn
                text: i18n.tr("Back")
                anchors { left: parent.left }
                onClicked: pagestack.push(page_contacts);
            }
            ListModel {
                id: conversationMessages
            }

            /* List view showing the messages */
            ListView {
                anchors {top: back_btn.bottom; left: parent.left; right: parent.right; bottom: newMessage_inpt.top }
                anchors.margins: units.gu(1)
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
                                text: type == "message" ? content : type
                            }
                            Label { //Status
                                text: (timestamp ? (new Date(timestamp*1000) + " ") : "" )
                                      +  (sent ? "sent " : "") + (delivered ? "delivered" : "")
                                font.italic: true
                                font.pointSize: 6
                            }
                        }
                        Image {
                            id: preview_img
                            anchors.margins: units.gu(1)
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
    ListModel {
        id: allMessages
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
            pagestack.push(page_login);
        }
        onConnectionStatusChanged: {
            Util.log("onConnectionStatusChanged " + status);
            page_contacts.state = status;
        }

        onAccountEnabledChanged: {
            Util.log("onAccountEnabledChanged " + enabled);
            accountEnabled_chk.checked = enabled;
            if(!enabled)
                pagestack.push(page_login);
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

        onMessage_error: {
            console.log("onMessage_error " + msgId + " jid: " + jid + " errorCode: "+ errorCode);
        }
        onDisconnected: {
            Util.log("OnDisconnected: " + reason);
            if(reason != "dbus_setup")
                page_contacts.state = "disconnected";
        }
        /* Messaging */
        onAudio_received: {
            console.log("onAudio_received");

            DB.addMessage({ "type": "audio", "url": url,
                              "jid": jid, "msgId": msgId, "size":  size,
                              "incoming": 1});
            DB.updateMessages();
        }
        onMessage_received: {
            console.log("OnMessage_received: " + msgId + " " + jid + " " + content + " " + timestamp + " " + wantsReceipt);
            DB.addMessage({ "type": "message", "content": content,
                              "jid": jid, "msgId": msgId, "timestamp":  timestamp,
                              "incoming": 1});
            DB.updateMessages();
        }
        onImage_received: {
            console.log("onImage_received " + url + " " + size);

            DB.addMessage({ "type": "image", "preview": preview,
                              "jid": jid, "msgId": msgId, /*"timestamp":  timestamp,*/
                              "size": size, "url" : url,
                              "incoming": 1});
            DB.updateMessages();
        }
        onVideo_received : {
            console.log("onImage_received " + url + " " + size);

            DB.addMessage({ "type": "video", "preview": preview,
                              "jid": jid, "msgId": msgId, /*"timestamp":  timestamp,*/
                              "size": size, "url" : url,
                              "incoming": 1});
            DB.updateMessages();
        }
        onLocation_received : {
            //
            console.log("onLocation_received " + name  + " " + latitude + " " + longitude);

            DB.addMessage({ "type": "location", "content": name, "preview": preview,
                              "jid": jid, "msgId": msgId, /*"timestamp":  timestamp,*/
                              "latitude": latitude, "longitude" : longitude,
                              "incoming": 1});
            DB.updateMessages();
        }
        onMessage_send_completed: {
            Util.log("onMessage_send_completed " + jid + " " + message + " " + msgId);
            DB.addMessage({ "type": "message", "content": message,
                              "jid": jid, "msgId": msgId, "timestamp": 0,
                              "incoming": 0, "sent": 0, "delivered": 0});
            DB.updateMessages();
        }
        onReceipt_messageDelivered: {
            console.log("OnReceipt_messageDelivered: " + jid + " " + msgId);
            DB.setDelivered(jid,msgId);
            DB.loadMessages();
        }
        onReceipt_messageSent: {
            console.log("OnReceipt_messageSent: " + jid + " " + msgId);
            DB.setSent(jid,msgId);
            DB.loadMessages();
        }
        onReceipt_visible: {
            Util.log("OnReceipt_visible: " + jid + " " + msgId);
        }
        onStatus_dirty: {
            Util.log("OnStatus_dirty");
        }
        onContact_gotProfilePicture: {
            Util.log("OnContact_gotProfilePicture: " + jid + " " + filename);
        }
        onContact_gotProfilePictureId: {
            Util.log("OnContact_gotProfilePictureId: " + jid + " " + pictureId);
        }
        onContact_paused: {
            Util.log("OnReceipt_visible: " + jid);
        }
        onContact_typing: {
            Util.log("OnContact_typing: " + jid);
        }
        /* Registration */
        onCode_request_response: {
            Util.log("onCode_request_response: " + status );
            if( status != 'sent' ) {
                //TODO: some error has occured, look into reason
                //possible reason: too_recent
            }
        }
        onCode_register_response: {
            Util.log("onCode_register_response: " + status);
            if(status == 'ok') {
                //password_txt.text = pw;
                //username_txt.text = countrycode_txt.text+username_reg_txt.text;
                set_account(countrycode_txt.text+username_reg_txt.text, pw);
            } else {
                //TODO: error msg
            }
        }

    }

    function getPreviewImage(id) {
        return DB.getPreviewImage(id);
    }
}
