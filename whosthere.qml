import QtQuick 2.0
import QtQuick.LocalStorage 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import Yowsup 1.0
import "whosthere.js" as WhosThere

Rectangle {
    id: root
    width: units.gu(60)
    height: units.gu(80)
    color: "lightgray"

    property real margins: units.gu(2)
    property real buttonWidth: units.gu(9)

    PageStack {
        id: pagestack
        anchors.fill: parent
        Component.onCompleted: {
            WhosThere.updateMessages();
            push(page_login)
        }
        Component.onDestruction: {
            yowsup.disconnect("");
        }

        Page {
            id: page_login
            visible: false
            title: "Login"
            anchors.fill: parent
            onVisibleChanged: WhosThere.fillCredentials();
            Column {
                anchors.fill: parent
                TextField {
                    id: username_txt
                    placeholderText: "Telephone number without leading + or 00"
                    width: parent.width; height: 28
                }
                TextField {
                    id: password_txt
                    placeholderText: "Password in base64"
                    width: parent.width; height: 28
                }
                Row {
                    Button {
                        id: login_btn
                        text: "Log in"
                        onClicked: {
                            if( username_txt.text == "" || password_txt.text == "")
                                return;

                            yowsup.login(username_txt.text, password_txt.text);
                            WhosThere.saveCredentials(username_txt.text, password_txt.text);
                        }
                    }
                    Button {
                        text: "Demo"
                        onClicked: {
                            allMessages.append({ "type": "message", "content": "Hi there",
                                                   "jid": "155556778317@s.whatsapp.net", "msgId": "1", "timestamp": 0,
                                                   "incoming": true, "sent": false, "delivered": false});
                            allMessages.append({ "type": "message", "content": "How are you doing?",
                                                   "jid": "155556778317@s.whatsapp.net", "msgId": "2", "timestamp": 2,
                                                   "incoming": false, "sent": true, "delivered": true});
                            allMessages.append({ "type": "message", "content": "Everything alright?",
                                                   "jid": "155556778317@s.whatsapp.net", "msgId": "3", "timestamp": 17,
                                                   "incoming": false, "sent": true, "delivered": false});

                            allMessages.append({ "type": "message", "content": "Hello!",
                                                   "jid": "155556777777@s.whatsapp.net", "msgId": "1", "timestamp": 0,
                                                   "incoming": true, "sent": false, "delivered": false});
                            allMessages.append({ "type": "message", "content": "I'm sending a text",
                                                   "jid": "155556777777@s.whatsapp.net", "msgId": "2", "timestamp": 2,
                                                   "incoming": true, "sent": false, "delivered": false});
                            allMessages.append({ "type": "message", "content": ".. and I'm answering",
                                                   "jid": "155556777777@s.whatsapp.net", "msgId": "3", "timestamp": 17,
                                                   "incoming": false, "sent": false, "delivered": false});
                            WhosThere.updateMessages();
                            pagestack.push(page_contacts);
                        }
                    }
                }
            }
        }
        /* page_contacts */
        Page {
            id: page_contacts
            visible: false
            title: "Contacts"
            anchors.fill: parent
            ListModel {
                id: contactsModel
            }
            ListView {
                anchors.fill: parent
                model: contactsModel
                delegate: ListItem.Subtitled {
                    text: jid + " ( time: " + timestamp + " )"
                    subText: "Last message: " + content
                    MouseArea {
                        anchors.fill: parent
                        onClicked: WhosThere.showConversation(jid);
                    }
                }
            }
            tools: ToolbarActions {
                Action {
                    text: "Logout"
                    iconSource: Qt.resolvedUrl("avatar_contacts_list.png")
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
            title: "Conversation with " + jid
            Button {
                id: back_btn
                text: "Back"
                anchors { left: parent.left }
                onClicked: pagestack.push(page_contacts);
            }
            ListModel {
                id: conversationMessages
            }
            ListView {
                anchors {top: back_btn.bottom; left: parent.left; right: parent.right; bottom: newMessage_inpt.top }
                model: conversationMessages
                delegate: ListItem.Subtitled {
                    //anchors {left: parent.left; right: parent.right }
                    //width: parent.width/2
                    Column {
                        x: incoming ? parent.width/2 : 0
                        Label { //Text
                            text: content
                        }
                        Label { //Status
                            text: (sent ? "sent " : "") + (delivered ? "delivered" : "")
                            font.italic: true
                            font.pointSize: 6
                        }
                    }
                }
            }
            TextField {
                id: newMessage_inpt
                placeholderText: "Type message"
                //color: "white"
                width: parent.width; height: 28
                anchors {bottom: parent.bottom; left: parent.left; right: parent.right }
                onAccepted: {
                    if( text === "" )
                        return;
                    var msgId = yowsup.message_send(page_conversation.jid, text)
                    console.log("Sent message has id " + msgId);

                    WhosThere.addMessage({ "type": "message", "content": text,
                                           "jid": page_conversation.jid, "msgId": msgId, "timestamp": 0,
                                           "incoming": false, "sent": false, "delivered": false});
                    text = "";
                }
            }
        }
    }
    ListModel {
        id: allMessages
    }
    Yowsup {
        id: yowsup
        onAuth_success: {
            console.log("auth success for " + username);
            ready();
            WhosThere.loadMessages();
            pagestack.push(page_contacts);
        }
        onAuth_fail: {
            //PopupUtils.open(errorPopover, yowsup);
            console.log("auth fail for " + username + ", reason: " + reason);
        }
        onMessage_error: {
            console.log("onMessage_error " + messageId + " jid: " + jid + " errorCode: "+ errorCode);
        }
        onDisconnected: {
            console.log("OnDisconnected: " + reason);
            pagestack.push(page_login);
        }
        onAudio_received: {
            console.log("onAudio_received");
        }
        onMessage_received: {
            console.log("OnMessage_received: " + msgId + " " + jid + " " + content + " " + timestamp + " " + wantsReceipt);
            if(wantsReceipt)
                message_ack(jid, msgId);
            WhosThere.addMessage({ "type": "message", "content": content,
                                   "jid": jid, "msgId": msgId, "timestamp": timestamp,
                                   "incoming": true, "sent": false, "delivered": false});
            WhosThere.updateMessages();
        }
        onReceipt_messageDelivered: {
            console.log("OnReceipt_messageDelivered: " + jid + " " + msgId);
            delivered_ack(jid, msgId);
            var msg = WhosThere.getMessage(jid, msgId);
            if(msg)
                msg.delivered = true;
            WhosThere.updateMessages();
        }
        onReceipt_messageSent: {
            console.log("OnReceipt_messageSent: " + jid + " " + msgId);
            var msg = WhosThere.getMessage(jid, msgId);
            if(msg)
                msg.sent = true;
            WhosThere.updateMessages();
        }
        onReceipt_visible: {
            console.log("OnReceipt_visible: " + jid + " " + msgId);
        }
        onStatus_dirty: {
            console.log("OnStatus_dirty");
        }
        onContact_gotProfilePicture: {
            console.log("OnContact_gotProfilePicture: " + jid + " " + filename);
        }
        onContact_gotProfilePictureId: {
            console.log("OnContact_gotProfilePictureId: " + jid + " " + pictureId);
        }
        onContact_paused: {
            console.log("OnReceipt_visible: " + jid);
        }
        onContact_typing: {
            console.log("OnContact_typing: " + jid);
        }

        onPing: {
            console.log("onPing: " + pingId);
            pong(pingId);
        }
    }
}
