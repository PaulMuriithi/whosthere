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

var db = openDB();

var presences = new Object;

function showConversation(jid) {
    page_conversation.jid = jid;
    loadConversation();
    pagestack.push(page_conversation);
}

function displayName(jid) {
    return "+" + jid.replace(/@s\.whatsapp\.net/, "");
}

function updateContacts() {
    contactsModel.clear();
    var contacts = [];
    db.transaction(
                function(tx) {
                    var rs = tx.executeSql("SELECT ct.jid, ct.pushName, ct.alias, ct.avatar, lastmsg.content, lastmsg.lastTime FROM Contacts ct, (SELECT max(timestamp) as lastTime, content, jid FROM Messages WHERE type = 'message' GROUP BY jid) lastmsg WHERE ct.jid = lastmsg.jid");
                    console.log("updateContacts results: " + rs.rows.length)
                        for(var i=0;i < rs.rows.length; ++i) {
                            for(var j in rs.rows.item(i))
                                console.log("Item: " + j + " = " + rs.rows.item(i)[j])
                            contacts.push({   "jid":      '' + rs.rows.item(i).jid,
                                              "pushName": '' + rs.rows.item(i).pushName,
                                              "alias":    '' + rs.rows.item(i).alias,
                                              "avatar":   '' + rs.rows.item(i).avatar,
                                              "lastMsg":   '' + rs.rows.item(i).content,
                                              "lastTime":   '' + rs.rows.item(i).lastTime,
                                               });
                        }

                });


    for(var i in contacts) {
        if( contacts[i]["jid"] in presences ) {
            contacts[i]["presence"] = presences[contacts[i]["jid"]];
        } else {
            contacts[i]["presence"] = "";
        }

        contactsModel.append(contacts[i]);
    }
}

function addContact(jid) {
    db.transaction(
                function(tx) {
                    tx.executeSql('INSERT INTO Contacts (jid) VALUES(?)', jid);
                });
}

function hasContact(jid) {
    var has;
    db.readTransaction(
                function(tx) {
                    var rs = tx.executeSql('SELECT jid FROM Contacts WHERE jid = ?', jid);
                    has = (rs.rows.length > 0);
                }
                );
    return has;
}

function openDB() {
    //NOTE: changeVersion does not have an effect until database is opened again
    var db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000,
            function(db) {
                db.transaction(
                    function(tx) {
                        tx.executeSql('CREATE TABLE Credentials(uid TEXT)');
                        tx.executeSql("INSERT INTO Credentials (uid) VALUES('')");
                        tx.executeSql("CREATE TABLE Messages(type TEXT NOT NULL, jid TEXT NOT NULL, msgId TEXT UNIQUE NOT NULL, "
                                     +"content TEXT DEFAULT '', preview BLOB DEFAULT '', url TEXT DEFAULT '', "
                                     +"size INT DEFAULT 0, timestamp TIMESTAMP DEFAULT 0, incoming INT DEFAULT 0, "
                                     +"sent INT DEFAULT 0, delivered INT DEFAULT 0, "
                                     +"longitude REAL DEFAULT 0, latitude REAL DEFAULT 0, name TEXT DEFAULT '', vcard TEXT DEFAULT '')");
                        tx.executeSql("CREATE TABLE Contacts (jid TEXT UNIQUE NOT NULL, pushName TEXT NOT NULL DEFAULT '', alias TEXT NOT NULL DEFAULT '', avatar BLOB)");
                    });
                db.changeVersion("","8");
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
    if(db.version == "5") {
        console.log("Updating db to version 6");
        db.changeVersion("5","6", function(tx) {
            tx.executeSql('ALTER TABLE Credentials ADD valid INT');
            var rs = tx.executeSql('SELECT * FROM Credentials');
            if(rs.rows.length == 0)
                tx.executeSql("INSERT INTO Credentials (username, password, uid, valid) VALUES('','','',0)");
            });
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);
        console.log("Now at version " + db.version);
    }
    if(db.version == "6") {
        console.log("Updating db to version 7");
        db.changeVersion("6","7", function(tx) {
            tx.executeSql("CREATE TABLE Contacts (jid TEXT UNIQUE NOT NULL, pushName TEXT NOT NULL DEFAULT '', alias TEXT NOT NULL DEFAULT '', avatar BLOB)");
            });
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);
        console.log("Now at version " + db.version);
    }
    if(db.version == "7") {
        console.log("Updating db to version 8");
        db.changeVersion("7","8", function(tx) {
            tx.executeSql("ALTER TABLE Messages ADD name TEXT DEFAULT ''");
            tx.executeSql("ALTER TABLE Messages ADD vcard TEXT DEFAULT ''");
            });
        db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000);
        console.log("Now at version " + db.version);
    }
    return db;
}

function loadConversation() {
    console.log("loadMessages");

    if(!page_conversation.jid)
        return;

    conversationMessages.clear();
    db.transaction(
                function(tx) {
                        var rs = tx.executeSql('SELECT * FROM Messages WHERE jid=?',page_conversation.jid);
                        //We cannot just do conversationMessages.append(rs.rows.item(i)), because
                        //sqlite does not keep our dataytypes
                        for(var i=0;i < rs.rows.length; ++i) {
                            //Don't use bool here, support for that is broken in ListModel
                            conversationMessages.append({   "type":      '' + rs.rows.item(i).type,
                                                   "jid":       '' + rs.rows.item(i).jid,
                                                   "msgId":     '' + rs.rows.item(i).msgId,
                                                   "content":   '' + rs.rows.item(i).content,
                                                   "preview":   '' + rs.rows.item(i).preview,
                                                   "url":       '' + rs.rows.item(i).url,
                                                   "size":      + rs.rows.item(i).size,
                                                   "timestamp": + rs.rows.item(i).timestamp,
                                                   "incoming":  + rs.rows.item(i).incoming,
                                                   "sent":      + rs.rows.item(i).sent,
                                                   "delivered": + rs.rows.item(i).delivered,
                                                   "longitude": + rs.rows.item(i).longitude,
                                                   "latitude":  + rs.rows.item(i).latitude,
                                                   "name":      '' + rs.rows.item(i).name,
                                                   "vcard":     '' + rs.rows.item(i).vcard
                                                   });
                        }

                });
}

function addMessage(msg) {

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
    loadConversation();
    //This will change the last received message
    updateContacts();
}

//Sets the delivered column to true
function setDelivered(jid,msgId) {
    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Messages SET "delivered"=1 WHERE jid = ? AND msgId = ?', [jid, msgId] );
                });
    loadConversation();
}

//Sets the sent column to true
function setSent(jid,msgId) {
    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Messages SET "sent"=1 WHERE jid = ? AND msgId = ?', [jid, msgId] );
                });
    loadConversation();
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

function getUID() {
    var uid;
    db.readTransaction(
                function(tx) {
                    var rs = tx.executeSql('SELECT uid FROM Credentials');
                    if(rs.rows.length == 0) {
                        console.log('No uid found!');
                    } else {
                        uid = rs.rows.item(0).uid;
                    }
                }
                );
    if(!uid || uid.length !== 32) {
        uid = createUID();
        setUID(uid);
    }
    return uid;
}

function setUID(uid) {
    console.log("setUID");

    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Credentials SET uid=?',
                                  [ uid ]);
                }
                );
}

function getPreviewImage(id) {
    console.log("getPreviewImage");

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
