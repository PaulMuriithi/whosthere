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

    var contacts = [];
    var i;
    for(i=0;i<allMessages.count;++i) {
        if(allMessages.get(i).jid === page_conversation.jid)
            conversationMessages.append(allMessages.get(i));
        contacts[allMessages.get(i).jid] = allMessages.get(i);
    }

    contactsModel.clear();
    for(i in contacts)
        contactsModel.append(contacts[i]);
}

function openDB() {
    //NOTE: changeVersion does not have an effect until database is opened again
    var db = LocalStorage.openDatabaseSync("WhosThere", "", "WhosThere Database", 1000000,
            function(db) {
                db.transaction(
                    function(tx) {
                        tx.executeSql('CREATE TABLE Credentials(username TEXT, password TEXT, uid TEXT, valid INT)');
                        tx.executeSql("INSERT INTO Credentials (username, password, uid, valid) VALUES('','','',0)");
                        tx.executeSql("CREATE TABLE Messages(type TEXT NOT NULL, jid TEXT NOT NULL, msgId TEXT UNIQUE NOT NULL, "
                                     +"content TEXT DEFAULT '', preview BLOB DEFAULT '', url TEXT DEFAULT '', "
                                     +"size INT DEFAULT 0, timestamp TIMESTAMP DEFAULT 0, incoming INT DEFAULT 0, "
                                     +"sent INT DEFAULT 0, delivered INT DEFAULT 0, "
                                     +"longitude REAL DEFAULT 0, latitude REAL DEFAULT 0)");
                    });
                db.changeVersion("","6");
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
    return db;
}

function loadMessages() {
    console.log("loadMessages");

    allMessages.clear();
    db.transaction(
                function(tx) {
                        var rs = tx.executeSql('SELECT * FROM Messages');
                        //We cannot just do allMessages.append(rs.rows.item(i)), because
                        //sqlite does not keep our dataytypes
                        for(var i=0;i < rs.rows.length; ++i)

                            allMessages.append({   "type":      '' + rs.rows.item(i).type,
                                                   "jid":       '' + rs.rows.item(i).jid,
                                                   "msgId":     '' + rs.rows.item(i).msgId,
                                                   "content":   '' + rs.rows.item(i).content,
                                                   "preview":   '' + rs.rows.item(i).preview,
                                                   "url":       '' + rs.rows.item(i).url,
                                                   "size":      + rs.rows.item(i).size,
                                                   "timestamp": + rs.rows.item(i).timestamp,
                                                   "incoming":  !! rs.rows.item(i).incoming,
                                                   "sent":      !! rs.rows.item(i).sent,
                                                   "delivered": !! rs.rows.item(i).delivered,
                                                   "longitude": + rs.rows.item(i).longitude,
                                                   "latitude":  + rs.rows.item(i).latitude
                                                   });

                });
    updateMessages();
}

function addMessage(msg) {
    allMessages.append(msg);

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
    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Messages SET "delivered"=1 WHERE jid = ? AND msgId = ?', [jid, msgId] );
                });
}

//Sets the sent column to true
function setSent(jid,msgId) {
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

function getUsername() {
    var username;
    db.readTransaction(
                function(tx) {
                    var rs = tx.executeSql('SELECT username FROM Credentials');
                    if(rs.rows.length == 0) {
                        console.log('No username found!');
                    } else {
                        username = rs.rows.item(0).username;
                    }
                }
                )
    return username;
}

function getPassword() {
    var password;
    db.readTransaction(
                function(tx) {
                    var rs = tx.executeSql('SELECT password FROM Credentials');
                    if(rs.rows.length == 0) {
                        console.log('No password found!');
                    } else {
                        password = rs.rows.item(0).password;
                    }
                }
                )
    return password;
}

function getCredentialsValid() {
    var valid;
    db.readTransaction(
                function(tx) {
                    var rs = tx.executeSql('SELECT valid FROM Credentials');
                    if(rs.rows.length == 0) {
                        console.log('No valid found!');
                    } else {
                        valid = rs.rows.item(0).valid;
                    }
                }
                )
    console.log("valid " + valid);
    return valid;
}

function setCredentialsValid(isValid) {
    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Credentials SET valid=?',
                                  [ isValid ]);
                }
                );
}

function setUsername(username) {
    console.log("setUsername");

    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Credentials SET username=?',
                                  [ username ]);
                }
                );
}

function setPassword(password) {
    console.log("setPassword");

    db.transaction(
                function(tx) {
                    tx.executeSql('UPDATE Credentials SET password=?',
                                  [ password ]);
                }
                );
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
