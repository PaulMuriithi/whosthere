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
    //anchors.centerIn: parent
    //anchors.fill: parent
    //color: "lightgray"
    Component.onDestruction: {
        whosthere.disconnect("");
    }

    Component.onCompleted: {
        Util.log("Component.onCompleted");
        pagestack.push(page_loading);
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
            onVisibleChanged: {
                if(visible) {
                    DB.loadMessages();
                    whosthere.connectDBus();
                }
            }
        }

        Page {
            id: page_login
            visible: false
            title: i18n.tr("Login/Register")
            anchors.fill: parent
            onVisibleChanged: {
                if(visible) {
                    username_txt.text = DB.getUsername();
                    password_txt.text = DB.getPassword();
                }
            }
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
                    Button {
                        anchors.margins: units.gu(1)
                        id: login_btn
                        text: i18n.tr("Log in")
                        onClicked: {
                            if( username_txt.text == "" || password_txt.text == "")
                                return;

                            whosthere.login(username_txt.text, password_txt.text);
                            DB.setUsername(username_txt.text);
                            DB.setPassword(password_txt.text);
                        }
                    }
                    Button {
                        anchors.margins: units.gu(1)
                        text: i18n.tr("Demo")
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
                        DB.setUsername(countrycode_txt.text+username_reg_txt.text);
                        //requesting the code invalidates the old password
                        DB.setPassword('');
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

                        whosthere.code_register(countrycode_txt.text, username_reg_txt.text, code, DB.getUID());
                    }
                }

                Label {
                    anchors.margins: units.gu(1)
                    text: i18n.tr("By connecting you agree to <a href='http://www.whatsapp.com/legal/#TOS'>Whatsapp's terms of service</a>");
                    onLinkActivated: Qt.openUrlExternally(link)
                    wrapMode: Text.WordWrap
                }
            }
        }
        /* page_contacts */
        Page {
            id: page_contacts
            visible: false
            title: i18n.tr("Contacts")
            anchors.fill: parent
            Label {
                anchors { left: parent.left; right: parent.right; top: parent.top; margins: units.gu(4) }
                visible: contactsModel.count == 0
                text: i18n.tr("You don't have any contacts yet. Receive a message from a friend!")
                wrapMode: Text.WordWrap
            }

            ListModel {
                id: contactsModel
            }
            ListView {
                anchors.fill: parent
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

        /* Init/Error */
        onAuth_success: {
            Util.log("auth success for " + username);
            ready();
            DB.setCredentialsValid(true);
            pagestack.push(page_contacts);
        }
        onAuth_fail: {
            console.log("auth fail for " + username + ", reason: " + reason);
            if(reason == "invalid") {
                DB.setCredentialsValid(false);
                pagestack.push(page_login);
            } else {
                console.log("onAuth_fail: Unhandled reason: " + reason)
            }
        }
        onMessage_error: {
            console.log("onMessage_error " + msgId + " jid: " + jid + " errorCode: "+ errorCode);
        }
        onDisconnected: {
            Util.log("OnDisconnected: " + reason);
            if(reason != "dbus_setup")
                pagestack.push(page_login);
        }
        onDbus_fail: {
            Util.log("onDbus_fail " + reason);
        }
        onDbus_connected: {
            Util.log("onDbus_connected");
            var valid = DB.getCredentialsValid();
            var username = DB.getUsername();
            var password = DB.getPassword();

            if(valid && username && password) {
                Util.log("Autologin with username " + username + " and password " + password);
                login(username, password);
            } else {
                pagestack.push(page_login);
            }
        }
        /* Messaging */
        onAudio_received: {
            console.log("onAudio_received");
            if(wantsReceipt)
                message_ack(jid, msgId);
            DB.addMessage({ "type": "audio", "url": url,
                              "jid": jid, "msgId": msgId, "size":  size,
                              "incoming": 1});
            DB.updateMessages();
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
        onMessage_send_completed: {
            Util.log("onMessage_send_completed " + jid + " " + message + " " + msgId);
            DB.addMessage({ "type": "message", "content": message,
                              "jid": jid, "msgId": msgId, "timestamp": 0,
                              "incoming": 0, "sent": 0, "delivered": 0});
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
        /* Registration */
        onCode_request_response: {
            console.log("onCode_request_response: " + status + " " + reason);
            if( status != 'send' ) {
                //TODO: some error has occured, look into reason
            }
        }
        onCode_register_response: {
            console.log("onCode_register_response: " + status + " " + reason);
            if(status == 'ok') {
                DB.setPassword(pw);
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
