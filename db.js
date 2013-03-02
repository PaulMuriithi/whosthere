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
                        tx.executeSql('CREATE TABLE Credentials(username TEXT, password TEXT, uid TEXT)');
                        tx.executeSql('CREATE TABLE Messages(type TEXT, jid TEXT, msgId TEXT, content TEXT, preview BLOB, url TEXT, size INT, timestamp TIMESTAMP, incoming BOOL, sent BOOL, delivered BOOL)');
                    });
                db.changeVersion("","3");
            });
    //After the database has been created fresh, its db.version == "" until we do openDatabaseSync() again
    if(db.version == "")
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);

    if(db.version == "") //Artifact from first release
        console.log("Database broken, please rm -R .local/share/whosthere")

    console.log('openDb with version '+ db.version);

    if(db.version == "2") {
        console.log("Updating db to 3");
        db.changeVersion("2","3", function(tx) {
            tx.executeSql('ALTER TABLE Messages ADD preview BLOB');
            });
        console.log("Now at version " + db.version);
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
                            allMessages.append(rs.rows.item(i));
                        }
                    } catch(err) {
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
                    tx.executeSql('INSERT INTO Messages (type, content, jid, msgId, timestamp, incoming, sent, delivered, preview, size, url ) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)',
                                  [ msg['type'], msg['content'], msg['jid'], msg['msgId'], msg['timestamp'],
                                  msg['incoming'], msg['sent'], msg['delivered'],
                                  msg['preview'], msg['size'], msg['url']]);
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
                    tx.executeSql('INSERT INTO Credentials (username, password, uid) VALUES(?, ?, ?)', [ username, password, uid ]);
                }
                )
}

function getPreviewImage(id) {
    console.log("saveCredentials");
    var db = openDB();

    var data;
    db.readTransaction(
                function(tx) {
                    var rs = tx.executeSql('SELECT preview FROM Messages WHERE msgId = ? AND type ="image"', id);
                    if(rs.rows.length == 0) {
                        console.log('No preview image found for id ' + id);
                    } else {
                        data = rs.rows.item(0).preview;
                    }
                }
                )
    console.log("data: " + data)
    return data;
}
