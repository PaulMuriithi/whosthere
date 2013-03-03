import QtQuick 2.0
import QtQuick.LocalStorage 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import WhosThere 1.0
import "db.js" as DB

MainView {
    id: root
    width: units.gu(42)
    height: units.gu(67)
    //anchors.centerIn: parent
    //anchors.fill: parent
    //color: "lightgray"

    PageStack {
        id: pagestack
        anchors.fill: parent
        Component.onCompleted: {
            DB.updateMessages();
            push(page_login)
        }
        Component.onDestruction: {
            whosthere.disconnect("");
        }

        Page {
            id: page_login
            visible: false
            title: "Login/Register"
            anchors.fill: parent
            onVisibleChanged: DB.fillCredentials();
            Column {
                anchors.fill: parent
                Label {
                    text: "Login"
                    font.underline: true
                    font.italic: true
                }
                Label {
                    text: "Login with your mobile number (including country code, excluding leading + or 00) and password. To obtain a password, see section 'Register' below."
                    wrapMode: Text.WordWrap
                    anchors { left: parent.left; right: parent.right }
                }
                TextField {
                    id: username_txt
                    placeholderText: "Telephone number without leading + or 00"
                    width: parent.width; height: units.gu(4)
                }
                TextField {
                    id: password_txt
                    placeholderText: "Password in base64"
                    width: parent.width; height: units.gu(4)
                }
                Row {
                    Button {
                        id: login_btn
                        text: "Log in"
                        onClicked: {
                            if( username_txt.text == "" || password_txt.text == "")
                                return;

                            whosthere.login(username_txt.text, password_txt.text);
                            DB.saveCredentials(username_txt.text, password_txt.text, uid_txt.text);
                        }
                    }
                    Button {
                        text: "Demo"
                        onClicked: {
                            allMessages.append({ "type": "message", "content": "Hi there",
                                                   "jid": "155556778317@s.whatsapp.net", "msgId": "1", "timestamp": 0,
                                                   "incoming": 1, "sent": 0, "delivered": 0});
                            allMessages.append({ "type": "message", "content": "How are you doing?",
                                                   "jid": "155556778317@s.whatsapp.net", "msgId": "2", "timestamp": 2,
                                                   "incoming": 1, "sent": 0, "delivered": 0});
                            allMessages.append({ "type": "message", "content": "Everything alright?",
                                                   "jid": "155556778317@s.whatsapp.net", "msgId": "3", "timestamp": 17,
                                                   "incoming": 0, "sent": 1, "delivered": 0});

                            allMessages.append({ "type": "message", "content": "Hello!",
                                                   "jid": "155556777777@s.whatsapp.net", "msgId": "1", "timestamp": 0,
                                                   "incoming": 1, "sent": 0, "delivered": 0});
                            allMessages.append({ "type": "message", "content": "I'm sending a text",
                                                   "jid": "155556777777@s.whatsapp.net", "msgId": "2", "timestamp": 2,
                                                   "incoming": 1, "sent": 0, "delivered": 0});
                            allMessages.append({ "type": "message", "content": ".. and I'm answering",
                                                   "jid": "155556777777@s.whatsapp.net", "msgId": "3", "timestamp": 17,
                                                   "incoming": 0, "sent": 0, "delivered": 0});
                            DB.updateMessages();
                            pagestack.push(page_contacts);
                        }
                    }
                }
                Label {
                    text: "Register"
                    font.italic: true
                    font.underline: true
                }
                Label {
                    text: "Registration is a two step process: First, enter your country code (e.g. '1' for US) and telephone number below (without country code and without leading 0), then tap 'Request code'. A code will be send via text to your number. Enter the code below and click 'Request password'. The password will appear in the password field above. Please save it somewhere! You can now login."
                    wrapMode: Text.WordWrap
                    anchors { left: parent.left; right: parent.right }
                }
                Row {
                    TextField {
                        id: countrycode_txt
                        placeholderText: "cc"
                        width: units.gu(10)
                    }
                    TextField {
                        id: username_reg_txt
                        placeholderText: "mobile number wo. cc"
                        //height: 28
                    }
                }
                Button {
                    text: "Request code"
                    width: units.gu(18)
                    onClicked: {
                        if(username_reg_txt.text == "" || countrycode_txt.text == "")
                            return;
                        console.log("onClicked: call code_request");
                        whosthere.code_request(countrycode_txt.text, username_reg_txt.text, uid_txt.text, true);
                        //requesting the code invalidates the old password
                        DB.saveCredentials(countrycode_txt.text+username_reg_txt.text, "", uid_txt.text);
                    }
                }
                TextField {
                    id: code_txt
                    placeholderText: "Code you got via text"
                    //height: 28
                }
                Button {
                    text: "Request password"
                    width: units.gu(22)
                    onClicked: {
                        if(username_reg_txt.text == "" || countrycode_txt.text == ""
                                || code_txt.text == "")
                            return;
                        console.log("onClicked: call code_register");
                        //Remove hyphon if it exists
                        var code = code_txt.text.replace('-','');

                        whosthere.code_register(countrycode_txt.text, username_reg_txt.text, code, uid_txt.text);
                    }
                }
                Label {
                    id: uid_txt
                }

                Label {
                    text: "By connecting you agree to <a href='http://www.whatsapp.com/legal/#TOS'>Whatsapp's terms of service</a>";
                    onLinkActivated: Qt.openUrlExternally(link)
                    wrapMode: Text.WordWrap
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
                        onClicked: DB.showConversation(jid);
                    }
                }
            }
            tools: ToolbarActions {
                Action {
                    text: "Logout"
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

            /* List view showing the messages */
            ListView {
                anchors {top: back_btn.bottom; left: parent.left; right: parent.right; bottom: newMessage_inpt.top }
                model: conversationMessages
                delegate: ListItem.Subtitled {
                    //anchors {left: parent.left; right: parent.right }
                    //width: parent.width/2
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if(url)
                                Qt.openUrlExternally(url)
                            else if(type == "location")
                                Qt.openUrlExternally("https://maps.google.com/maps?q="+latitude+","+longitude);
                        }
                    }
                    Column {
                        x: incoming ? parent.width/2 : 0
                        Label { //Text
                            text: type == "message" ? content : type
                        }
                        Image {
                            //Seems that the preview images are always 100x75
                            width: preview ? (sourceSize.height > sourceSize.width ? units.gu(6) : units.gu(8)) : 0
                            height: preview ? (sourceSize.height > sourceSize.width ? units.gu(8) : units.gu(6)) : 0
                            visible: preview
                            source: preview ? "image://drawable/" + msgId : ""
                        }
                        Label { //Status
                            text: (timestamp ? (new Date(timestamp*1000) + " ") : "" )
                                  +  (sent ? "sent " : "") + (delivered ? "delivered" : "")
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
                width: parent.width; height: units.gu(4)
                anchors {bottom: parent.bottom; left: parent.left; right: parent.right }
                onAccepted: {
                    if( text === "" )
                        return;
                    var msgId = whosthere.message_send(page_conversation.jid, text)
                    console.log("Sent message has id " + msgId);

                    DB.addMessage({ "type": "message", "content": text,
                                      "jid": page_conversation.jid, "msgId": msgId, "timestamp": 0,
                                      "incoming": 0, "sent": 0, "delivered": 0});
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

        onAuth_success: {
            console.log("auth success for " + username);
            ready();
            DB.loadMessages();
            pagestack.push(page_contacts);
        }
        onAuth_fail: {
            //PopupUtils.open(errorPopover, whosthere);
            console.log("auth fail for " + username + ", reason: " + reason);
        }
        onMessage_error: {
            console.log("onMessage_error " + msgId + " jid: " + jid + " errorCode: "+ errorCode);
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
            DB.addMessage({ "type": "message", "content": content,
                              "jid": jid, "msgId": msgId, "timestamp":  timestamp,
                              "incoming": 1});
            DB.updateMessages();
        }
        onImage_received: {
            console.log("onImage_received " + url + " " + size);

            if(wantsReceipt)
                message_ack(jid, msgId);
            DB.addMessage({ "type": "image", "preview": preview,
                              "jid": jid, "msgId": msgId, /*"timestamp":  timestamp,*/
                              "size": size, "url" : url,
                              "incoming": 1});
            DB.updateMessages();
        }
        onVideo_received : {
            console.log("onImage_received " + url + " " + size);

            if(wantsReceipt)
                message_ack(jid, msgId);
            DB.addMessage({ "type": "video", "preview": preview,
                              "jid": jid, "msgId": msgId, /*"timestamp":  timestamp,*/
                              "size": size, "url" : url,
                              "incoming": 1});
            DB.updateMessages();
        }
        onLocation_received : {
            //
            console.log("onLocation_received " + name  + " " + latitude + " " + longitude);

            if(wantsReceipt)
                message_ack(jid, msgId);
            DB.addMessage({ "type": "location", "content": name, "preview": preview,
                              "jid": jid, "msgId": msgId, /*"timestamp":  timestamp,*/
                              "latitude": latitude, "longitude" : longitude,
                              "incoming": 1});
            DB.updateMessages();
        }
        onReceipt_messageDelivered: {
            console.log("OnReceipt_messageDelivered: " + jid + " " + msgId);
            delivered_ack(jid, msgId);
            DB.setDelivered(jid,msgId);
            DB.loadMessages();
        }
        onReceipt_messageSent: {
            console.log("OnReceipt_messageSent: " + jid + " " + msgId);
            DB.setSent(jid,msgId);
            DB.loadMessages();
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
        onCode_request_response: {
            console.log("onCode_request_response: " + status + " " + reason);
            if( status != 'send' ) {
                //TODO: some error has occured, look into reason
            }
        }
        onCode_register_response: {
            console.log("onCode_register_response: " + status + " " + reason);
            if(status == 'ok') {
                DB.saveCredentials(countrycode_txt.text+username_reg_txt.text, pw, uid_txt.text);
                password_txt.text = pw;
                username_txt.text = countrycode_txt.text+username_reg_txt.text;
            } else {
                //TODO: error msg
            }
        }
    }
    function getPreviewImage(id) {
        return DB.getPreviewImage(id);
    }
}
