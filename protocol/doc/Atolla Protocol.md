# Atolla Protocol 1.0
This document describes the protocol that the Atolla project uses for
communications between Atolla devices and their clients.

## Introduction
The protocol enables the client to connect to devices and stream buffered light
information. The protocol is binary, efficient, quite simple and does not assume
the same native byte order for the communicating peers. It may be used on top of
both unreliable protocols like UDP and reliable protocols like TCP.

## Endianness
Multi-byte integers are always transmitted in little endian byte order. Hence,
bytes within an integer with a lower byte offset are more significant.
Implementations on big endian based systems must convert between internal and
network representations when sending or receiving.

## Data types
The protocol is comprised of different messages that are defined by means of the
following integral data types:

| Type   | Byte length   | Description                                             |
|----------------------------------------------------------------------------------|
| uint8  | 1             | Unsigned integer, ranges from 0 to 255                  |
| uint16 | 2             | Unsigned integer, little-endian, ranges from 0 to 65535 |
| data   | 2 up to 65537 | Dynamically-sized data – Comprised of an uint16 definining length of the payload in bytes (excluding the length bytes themselves), followed by that exact number of extra bytes |

## Messages
Using the atolla protocol, the peers communicate through messages. The following
format applies to all types of messages in the protocol:

| Byte ranges, 0-based | Data type  | Purpose           |
|-------------------------------------------------------|
| 0                    | uint8      | Message type      |
| 1 – 2                | uint16     | Message ID        |
| 3 – 4+               | data       | Message payload   |

The message type provides information to the receiver on how to interpret the
message and its payload.

The message ID is a connection-local, direction-local, pseudo-unique identifier
for the message. It increments with each message sent to the other peer. Hence,
it can be used to make assumptions about what message was sent first. Also,
older messages sent by the other peer can be referenced using their ID. The
message ID will, due to its limited length, frequently overflow in longer
communications. The implementation may use heuristics including guesses based on
when a message was received to decide which message was probably sent first or
which message was meant when referenced.

The payload contains any extra information required to interpret the message.
For example, a ENQUEUE message must contain light states in the payload, while
some other messages may have a zero-length payload (note that the payload
length must still be provided).

Note that the maximum payload length is limited by the length of an uint16. No
message can ever be larger than 65535 of payload plus 3 bytes of header and 2
bytes of payload length, yielding 65540 bytes.

Since the length of a message is encoded within the message, multiple
independent can be sent in the same block of memory. E.g. one UDP datagram may
hold multiple messages. This also makes sending messages through a streaming
connection possible.

Descriptions of message types will frequently leave out bytes 1 through 4, since
they work the same way for all types of messages.

### BORROW – Acquire device and provide configuration
This message is sent by clients to communicate to the device its desire to
control the light state.

#### Composition
The top level data is the same as for all messages, except that the length is
known beforehand:

| Byte ranges, 0-based | Data type  | Purpose                  |
|--------------------------------------------------------------|
| 0                    | uint8      | Message type, always 0   |
| 1 – 2                | uint16     | Message ID               |
| 3 – 6                | data       | Payload                  |

The payload is organized as follows:

| Byte ranges, 0-based | Data type  | Purpose                  |
|--------------------------------------------------------------|
| 3 – 4                | uint16     | Always 2, payload length |
| 5                    | uint8      | Frame length in ms       |
| 6                    | uint8      | Buffer length            |

#### Purpose
The borrow process serves three purposes. Firstly, it ensures that no two
clients can access a device at the same time. Secondly, it serves as a handshake
process, meaning the device acknowledges to the sender that it is there and will
honor requests from the sender. Furthermore, essential parameters can be
transmitted while borrowing. Currently, frame duration and buffer length are
transmitted with the BORROW message.

To overcome lag problems associated with WiFi, the device will utilize a
buffering technique to save some frames into the future. Upon borrowing, the
device will provide space for a buffer capable of holding at least as much
frames as requested by the client. When the client then enqueues frames, they
will be saved into the buffer. When the configured minimum amount of bufferable
frames has been enqueued, the device will start dequeuing frames from the
buffer with the frequency implied by the sent frame length, keeping the same
color if no newer color is available yet.

### LENT – Confirm a live connection
This message confirms that the device is available and ready for further
messages. It contains a zero-length payload.

After the initial lending process, more LENT messages will be sent in regular
intervals. Devices should at least every five seconds send a LENT message to
confirm they are still available.

The message type byte is 1 for LENT messages.

### ENQUEUE – Enqueue a light state
After having successfully borrowed a device, saves a frame into the buffer to
be shown later.

#### Composition

| Byte ranges, 0-based | Data type  | Purpose                  |
|--------------------------------------------------------------|
| 0                    | uint8      | Message type, always 2   |
| 1 – 2                | uint16     | Message ID               |
| 3 – 5+               | data       | Payload                  |


#### Purpose
In case of non-zero frame length and buffer length, the frame in the payload
will be enqueued and shown later.

In case of a zero frame length or buffer length, the frame will immediately be
shown.

The frame is encoded as a color-interleaved, one-dimensional bitmap with a color depth
of 8 bits per channel. Put differently, the frame is an array of three-byte colors in
red-green-blue order, with each color using up one byte of space, with higher
values being brighter and a value of zero is completely off. If the frame length
is lower than the physical number of light outputs in the device, the last color
(the last three bytes) is repeated until all outputs have a color. If the frame
length is higher, superfluous colors are silently ignored.

The payload size for all messages is limited to 65535 bytes, due to its size
being specified with a 16 bit integer. Hence, a enqueue message can never hold
more than 21845 colors.

### FAIL - Communicate error conditions
A fail message communicates back to the client that a previous message could not
be interpreted as intended.

#### Composition
The basic data layout remains the same.

| Byte ranges, 0-based | Data type  | Purpose                  |
|--------------------------------------------------------------|
| 0                    | uint8      | Message type, always 255 |
| 1 – 2                | uint16     | Message ID               |
| 3 – 7                | data       | Payload                  |

The payload is organized in this way:

| Byte ranges, 0-based | Data type  | Purpose                       |
|-------------------------------------------------------------------|
| 3 – 4                | uint16     | Always 3, payload length      |
| 5 – 6                | uint16     | Message ID of causing message |
| 7                    | uint8      | Error code                    |

The following error codes are currently defined:

| Error code | Meaning                                                   |
|------------------------------------------------------------------------|
| 0          | Tried to enqueue a frame, but the device is not borrowed to the current client. The client either never sent a BORROW or another client has borrowed it now. |

#### Purpose
Communicates to the client that one of its sent messages could not be
interpreted as intended. Successful conditions, with the exception of successful
borrowing, are never communicated back and assumed to be the default case. Fails
communicate the type of error and which message caused it, but no human-readable
descriptions of the error.

## Changelog

| Version      | Changes                          |
|-------------------------------------------------|
| 1.0          | Initial version of this document |
