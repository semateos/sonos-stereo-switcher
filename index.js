

var serialport = require("serialport");
var SerialPort = serialport.SerialPort;
var Queue = require('queuejs');

var config = {
  "portName": "/dev/cu.usbmodem1421"
}

var EventEmitter = require('events');
var util = require('util');

function MyEmitter() {
  EventEmitter.call(this);
}

util.inherits(MyEmitter, EventEmitter);


var remote = {

  serialPort: new SerialPort(config.portName, {
    baudrate: 57600,
    dataBits: 8,
    parity: 'none',
    stopBits: 1,
    flowControl: false,
    parser: serialport.parsers.readline('\n')
  }, false),

  emitter: new MyEmitter(),

  commandQueue: new Queue(),

  paused: true,

  queueCommand: function(commandString){

    this.commandQueue.enq(commandString);

    console.log('queue length:', this.commandQueue.size());
  },

  sendNextCommand: function(){

    if(this.commandQueue.size() > 0){

      var command = this.commandQueue.deq();

      this.serialPort.write(command + "\n", function(err, results) {

        console.log('sent >', command);

        if(err){

          console.log('err ' + err);

        }else{

          console.log('results ' + results);
        }

      });
    }
  },

  start: function(){

    this.paused = false;
  },

  pause: function(){

    this.paused = true;
  },

  //** setup and connect to device **//

  connect: function(callback){

    var self = this;

    //send the next command in the queue when device responds 'ready'
    this.emitter.on('ready', function(){

      //only if the queue isn't paused
      if(!self.paused){

        self.sendNextCommand();
      }
    });

    //read data from the serial port
    this.serialPort.on('data', function(data){

      var receivedData = data.toString();

      console.log(receivedData);

      //emit ready event when we read 'ready' on the port
      if(receivedData.indexOf("READY") != -1){

        self.emitter.emit('ready');
      }

    });

    this.serialPort.on("open", function(){

      console.log('serial port open');

    });

    //open serial connection and call the callback

    try{

      this.serialPort.open(function(err){

        callback(err);

      });

    }catch(e){

      callback(err);
    }

  }


}

console.log('testing connection...');

remote.connect(function(err){

  if (err) {

    return console.log('Error connecting: ', err.message);
  }

  remote.queueCommand("testing testeroo");

  remote.start();

});
