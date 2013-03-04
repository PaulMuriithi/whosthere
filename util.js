.pragma library

function log(msg) {
    console.log( "[" +  Qt.formatTime(new Date(), "hh:mm:ss.zzz") + "] " + msg);
}
