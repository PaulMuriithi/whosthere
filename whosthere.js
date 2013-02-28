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

function loadMessages() {
    console.log("loadMessages");
    var db = LocalStorage.openDatabaseSync("WhosThere", "1.0", "WhosThere Database", 1000000);
    //db.changeVersion(from, to, callback(tx))

    db.transaction(
                function(tx) {
                    //try {
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
                    /*}
                    catch(err) {
                        console.log("Could not open database. Maybe this is the first run? Error: " + err);
                    }*/
                });
    updateMessages();
}

function addMessage(msg) {
    allMessages.append(msg);

    var db = LocalStorage.openDatabaseSync("WhosThere", "1.0", "WhosThere Database", 1000000);

    db.transaction(
                function(tx) {
                    // Create the database if it doesn't already exist
                    tx.executeSql('CREATE TABLE IF NOT EXISTS Messages(type TEXT, content TEXT, jid TEXT, msgId TEXT, timestamp INT, incoming BOOL, sent BOOL, delivered BOOL)');
                    tx.executeSql('INSERT INTO Messages VALUES(?, ?, ?, ?, ?, ?, ?, ?)',
                                  [ msg['type'], msg['content'], msg['jid'], msg['msgId'], msg['timestamp'],
                                  msg['incoming'], msg['sent'], msg['delivered']]);
                })
}

function fillCredentials() {
    console.log("fillCredentials");
    var db = LocalStorage.openDatabaseSync("WhosThere", "1.0", "WhosThere Database", 1000000);

    db.transaction(
                function(tx) {
                    try {
                        var rs = tx.executeSql('SELECT * FROM Credentials');

                        if(rs.rows.length > 1)
                            console.log("Error: multiple credentials found");

                        if(rs.rows.length > 0) {
                            username_txt.text = rs.rows.item(0).username;
                            password_txt.text = rs.rows.item(0).password;
                        }
                    } catch(err) {

                    }

                }
                )
}

function saveCredentials(username, password) {
    console.log("saveCredentials");
    var db = LocalStorage.openDatabaseSync("WhosThere", "1.0", "WhosThere Database", 1000000);

    db.transaction(
                function(tx) {
                    // Create the database if it doesn't already exist
                    tx.executeSql('CREATE TABLE IF NOT EXISTS Credentials(username TEXT, password TEXT)');

                    tx.executeSql('DELETE FROM Credentials');
                    tx.executeSql('INSERT INTO Credentials VALUES(?, ?)', [ username, password ]);
                }
                )
}
