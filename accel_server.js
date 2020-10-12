var net = require('net');
var server = net.createServer();
const { Duplex } = require('stream');

class ChunkStream extends Duplex {
  constructor(chunk_length, options) {
    super(options);
    if (!chunk_length) {
      throw new Error('Need chunk length');
    }
    this.chunk_length = chunk_length;
    this.write_buffer = Buffer.alloc(0);
    this.is_reading = false;
  }

  _write(chunk, encoding, cb) {
    this.write_buffer = Buffer.concat([this.write_buffer, chunk]);
    cb();
    this.go_read();
  }

  _read(size) {
    this.is_reading = true;
    while (this.go_read()) {}
  }

  go_read() {
    if (!this.is_reading) {
      return false;
    }
    if (this.write_buffer.length >= this.chunk_length) {
      // slice slow..
      if (!this.push(this.write_buffer.slice(0, this.chunk_length))) {
        this.is_reading = false;
        return false;
      }
      this.write_buffer = this.write_buffer.slice(this.chunk_length);
      return true;
    }
    return false;
  }
}

const createCsvWriter = require('csv-writer').createObjectCsvWriter;
const csvWriter = createCsvWriter({
    path: 'entries.csv',
    header: [
        {id: 'x', title: 'x'},
        {id: 'y', title: 'y'},
        {id: 'z', title: 'z'},
    ]
});

server.on('connection', handleConnection);
server.listen(9000, function() {    
  console.log('server listening to %j', server.address());  
});

function to_signed_int(byteA, byteB) {
  const sign = byteA & (1 << 7);
  let x = (((byteA & 0xFF) << 8) | (byteB & 0xFF));
  if (sign) {
     x = 0xFFFF0000 | x;
  }
  return x;
}

let rec_data = [];
function handleConnection(conn) {    
  const duplex = new ChunkStream(7);
  duplex.on('data', (data) => {
    const skipped_data_count = data[0];
    const x = to_signed_int(data[1], data[2]);
    const y = to_signed_int(data[3], data[4]);
    const z = to_signed_int(data[5], data[6]);
    csvWriter.writeRecords([{x, y, z}]).then(() => {
      console.log('skipped', skipped_data_count, 'data', x, y, z);
    });;
  });
  var remoteAddress = conn.remoteAddress + ':' + conn.remotePort;  
  console.log('new client connection from %s', remoteAddress);
  conn.once('close', onConnClose);  
  conn.on('error', onConnError);
  conn.pipe(duplex);
  function onConnClose() {  
    console.log('connection from %s closed', remoteAddress);  
  }
  function onConnError(err) {  
    console.log('Connection %s error: %s', remoteAddress, err.message);  
  }  
}
