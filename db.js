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
    //NOTE: changeVersion does not have an effect until database is opened again
    var db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000,
            function(db) {
                db.transaction(
                    function(tx) {
                        tx.executeSql('CREATE TABLE Credentials(username TEXT, password TEXT, uid TEXT)');
                        tx.executeSql("CREATE TABLE Messages(type TEXT NOT NULL, jid TEXT NOT NULL, msgId TEXT UNIQUE NOT NULL, "
                                     +"content TEXT DEFAULT '', preview BLOB DEFAULT '', url TEXT DEFAULT '', "
                                     +"size INT DEFAULT 0, timestamp TIMESTAMP DEFAULT 0, incoming INT DEFAULT 0, "
                                     +"sent INT DEFAULT 0, delivered INT DEFAULT 0, "
                                     +"longitude REAL DEFAULT 0, latitude REAL DEFAULT 0)");
                    });
                db.changeVersion("","5");
            });
    if(db.version == "") //changeVersion did not take effect yet
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);

    console.log('openDb with version '+ db.version);

    if(db.version == "") { //Artifact from first release
        console.log("Updating db to version 2");
        db.changeVersion("","2", function(tx) {
            tx.executeSql('ALTER TABLE Messages ADD url TEXT');
            tx.executeSql('ALTER TABLE Messages ADD size INT');
            });
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);
        console.log("Now at version " + db.version);
    }
    if(db.version == "2") {
        console.log("Updating db to version 3");
        db.changeVersion("2","3", function(tx) {
            tx.executeSql('ALTER TABLE Messages ADD preview BLOB');
            });
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);
        console.log("Now at version " + db.version);
    }
    if(db.version == "3") {
        console.log("Updating db to version 4");
        db.changeVersion("3","4", function(tx) {
            tx.executeSql('ALTER TABLE Messages ADD longitude REAL');
            tx.executeSql('ALTER TABLE Messages ADD latitude REAL');
            });
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);
        console.log("Now at version " + db.version);
    }
    if(db.version == "4") {
        console.log("Updating db to version 5");
        db.changeVersion("4","5", function(tx) {
            tx.executeSql('ALTER TABLE Messages RENAME TO oldMessages');
            tx.executeSql("CREATE TABLE Messages(type TEXT NOT NULL, jid TEXT NOT NULL, msgId TEXT UNIQUE NOT NULL, "
                         +"content TEXT DEFAULT '', preview TEXT DEFAULT '', url TEXT DEFAULT '', "
                         +"size INT DEFAULT 0, timestamp TIMESTAMP DEFAULT 0, incoming INT DEFAULT 0, "
                         +"sent INT DEFAULT 0, delivered INT DEFAULT 0, "
                         +"longitude REAL DEFAULT 0, latitude REAL DEFAULT 0)");
            tx.executeSql("INSERT INTO Messages (type, content, jid, msgId, timestamp, incoming, sent, delivered, "
                         +"preview, size, url, latitude, longitude ) "
                         +"SELECT type, content, jid, msgId, timestamp, incoming, sent, delivered, "
                         +"preview, size, url, latitude, longitude FROM oldMessages")
            tx.executeSql('DROP TABLE oldMessages');
            });
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);
        console.log("Now at version " + db.version);
    }
    return db;
}

function loadMessages() {
    console.log("loadMessages");
    var db = openDB();

    allMessages.clear();
    db.transaction(
                function(tx) {
                        var rs = tx.executeSql('SELECT * FROM Messages');
                        for(var i=0;i < rs.rows.length; ++i)
                            allMessages.append(rs.rows.item(i));
                });
    updateMessages();
}

function addMessage(msg) {
    allMessages.append(msg);

    var db = openDB();

    // 1:1 mapping of keys and values of msg to columns and values in SQL
    db.transaction(
                function(tx) {
                    var columns = '';
                    var valuesp = '';
                    var values = [];
                    for(var i in msg) {
                        if(columns.length == 0) {
                            columns = i;
                            valuesp = '?';
                        } else {
                            columns += ', ' + i;
                            valuesp += ', ?';
                        }
                        values.push(msg[i])
                    }
                    tx.executeSql('INSERT INTO Messages (' + columns + ') VALUES(' + valuesp + ')', values);
                })
}

//Sets the delivered column to true
function setDelivered(jid,msgId) {
    var db = openDB();

    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Messages SET "delivered"=1 WHERE jid = ? AND msgId = ?', [jid, msgId] );
                });
}

//Sets the sent column to true
function setSent(jid,msgId) {
    var db = openDB();

    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Messages SET "sent"=1 WHERE jid = ? AND msgId = ?', [jid, msgId] );
                });
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
                    var rs = tx.executeSql('SELECT preview FROM Messages WHERE msgId = ?', id);
                    if(rs.rows.length == 0) {
                        console.log('No preview image found for id ' + id);
                    } else {
                        data = rs.rows.item(0).preview;
                    }
                }
                )
    return data;
}
