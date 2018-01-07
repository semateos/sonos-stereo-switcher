const SerialPort = require('serialport');
const AsyncPolling = require('async-polling');
const getJSON = require('get-json')
const Readline = SerialPort.parsers.Readline;
const parser = new Readline();

var port = false;
var stereoMode = '';
var lastPlayState = '';

var getZoneStatus = function(zoneName, callback){

    var url = 'http://mini:5005/' + zoneName + '/state';

    getJSON(url, callback);
}


//open serial connection to Arduino
try{

    port = new SerialPort('/dev/cu.usbmodem1421', {
      baudRate: 9600
    });

    port.pipe(parser);

    var connected = false;

    var respond = function(data){

      console.log(data);

      if(data.includes('Serial Connected!')){

        connected = true;
      }
    }

    parser.on('data', respond);

}catch(error){

    console.log(error);
}

var switchToMode = function(mode){

  port.write(mode + '\n', function(err) {

    if (err) {

      return console.log('Error on write: ', err.message);
    }

    console.log('Sent ' + mode + ' Command');

    stereoMode = mode;

    return false;
  });

  port.drain();
}



// Open errors will be emitted as an error event
port.on('error', function(err) {
  console.log('Error: ', err.message);
});


//start polling 3rd floor status
var polling = AsyncPolling(function (end) {

    getZoneStatus('3rd Floor',function (error, result) {

        if (error) {
            // Notify the error:
            end(error)
            return;
        }

        // Then send it to the listeners:
        end(null, result);
    });

}, 500);

polling.on('error', function (error) {
    // The polling encountered an error, handle it here.

    console.error(error);
});

polling.on('result', function (result) {
    // The polling yielded some result, process it here.

    if(result && result.playbackState){

        //console.log(result.playbackState);

        if(result.playbackState == 'PLAYING' && lastPlayState != 'PLAYING'){

          switchToMode('CD');
        }

        lastPlayState = result.playbackState;
    }

});

polling.run(); // Let's start polling.