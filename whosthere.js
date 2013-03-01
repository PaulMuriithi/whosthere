function showConversation(jid) {
    page_conversation.jid = jid;
    updateMessages();
    pagestack.push(page_conversation);
}

function getMessage(jid, msgId) {
    for(var i=0;i<allMessages.count;++i)
        if(allMessages.get(i).jid === jid && allMessages.get(i).msgId === msgId)
            return allMessages.get(i);
    return undefined;
}

//allMessages list has been changed, propagate
function updateMessages() {
    console.log("updateMessages");
    conversationMessages.clear();

    var contacts = new Array();
    var i;
    for(i=0;i<allMessages.count;++i) {
        if(allMessages.get(i).jid === page_conversation.jid)
            conversationMessages.append(allMessages.get(i));
        console.log('Processing '+ allMessages.get(i).jid);
        contacts[allMessages.get(i).jid] = allMessages.get(i);
    }

    contactsModel.clear();
    for(i in contacts) {
        console.log("Append to contact: " + i);
        contactsModel.append(contacts[i]);
    }

}

function openDB() {
    var db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000,
            function(db) {
                db.transaction(
                    function(tx) {
                        tx.executeSql('CREATE TABLE IF NOT EXISTS Credentials(username TEXT, password TEXT, uid TEXT)');
                        tx.executeSql('CREATE TABLE IF NOT EXISTS Messages(type TEXT, content TEXT, jid TEXT, msgId TEXT, timestamp INT, incoming BOOL, sent BOOL, delivered BOOL)');
                    });
            });

    if(db.version == "1.0") {
        console.log("Updating db to 1.1");
        db.changeVersion("1.0","1.1", function(tx) {
            tx.executeSql('ALTER TABLE Credentials ADD uid TEXT');
            });
    }
    return db;
}

function loadMessages() {
    console.log("loadMessages");
    var db = openDB();
    //db.changeVersion(from, to, callback(tx))

    db.transaction(
                function(tx) {
                    try {
                        var rs = tx.executeSql('SELECT * FROM Messages');
                        for(var i=0;i < rs.rows.length; ++i) {
                            console.log("loaded message " + rs.rows.item(i).jid);
                            allMessages.append({"type": rs.rows.item(i).type,
                                                   "content": rs.rows.item(i).content,
                                                   "jid": rs.rows.item(i).jid,
                                                   "msgId": rs.rows.item(i).msgId,
                                                   "timestamp": rs.rows.item(i).timestamp,
                                                   "incoming": !!rs.rows.item(i).incoming,
                                                   "sent": !!rs.rows.item(i).sent,
                                                   "delivered": !!rs.rows.item(i).delivered,

                                                });
                        }
                    }
                    catch(err) {
                        console.log("Could not open database. Maybe this is the first run? Error: " + err);
                    }
                });
    updateMessages();
}

function addMessage(msg) {
    allMessages.append(msg);

    var db = openDB();

    db.transaction(
                function(tx) {
                    tx.executeSql('INSERT INTO Messages VALUES(?, ?, ?, ?, ?, ?, ?, ?)',
                                  [ msg['type'], msg['content'], msg['jid'], msg['msgId'], msg['timestamp'],
                                  msg['incoming'], msg['sent'], msg['delivered']]);
                })
}

function createUID() {
    var s = [];
    var hexDigits = "0123456789abcdef";
    for (var i = 0; i < 32; i++) {
        s[i] = hexDigits.substr(Math.floor(Math.random() * 0x10), 1);
    }
    s = s.join('');
    console.log('Created uid ' + s);
    return s;
}

function fillCredentials() {
    console.log("fillCredentials");
    var db = openDB()

    db.transaction(
                function(tx) {
                    try {
                        var rs = tx.executeSql('SELECT * FROM Credentials');

                        if(rs.rows.length > 1)
                            console.log("Error: multiple credentials found");

                        if(rs.rows.length > 0) {
                            username_txt.text = rs.rows.item(0).username;
                            password_txt.text = rs.rows.item(0).password;
                            if(rs.rows.item(0).uid && rs.rows.item(0).uid.length == 32)
                                uid_txt.text = rs.rows.item(0).uid;
                        }
                    } catch(err) {
                        console.log("Could not open database. Maybe this is the first run? Error: " + err);
                    }
                    if(uid_txt.text.length != 32)
                        uid_txt.text = createUID();
                }
                )
}

function saveCredentials(username, password, uid) {
    console.log("saveCredentials");
    var db = openDB();

    db.transaction(
                function(tx) {
                    tx.executeSql('DELETE FROM Credentials');
                    tx.executeSql('INSERT INTO Credentials VALUES(?, ?, ?)', [ username, password, uid ]);
                }
                )
}
