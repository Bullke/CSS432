////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Nenad Bulicic                                                                           \\
// CSS 432 - Program 2: Sliding Window                                                     \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// This assignment implements three different kinds of UDP data transfer algorithms        \\
// (i.e. protocols): unreliable transfer, stop-and-wait, and sliding window. The program   \\
// itself first creates a socket object (defined by the UdpSocket file), creates a         \\
// 1460-byte message to transfer, and then transfers the created message using one of the  \\
// protocol test cases while recording the transfer performance over a 1Gbps network.      \\
////////////////////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#include <stdio.h>
#include <iostream>
#include <vector>
#include "Timer.h"
#include "UdpSocket.h"

#define TIMEOUT 1500 // Prespecified timeout window of 1500 us

// Repeats sending message[] and receiving an acknowledgment at a client side
// max (=20,000) times using the sock object. If the client cannot receive an
// acknowledgment immediately, it should start a timer. If a timeout (= 1500 usec)
// has happened, the client must resend the same message. The function must count
// the number of messages retransmitted and return it to the main function as its
// return value.
int clientStopWait( UdpSocket &sock, const int max, int message[] )
{
    int retransmitted = 0;
    Timer timer;

    // Loop until all 20000 frames have been sent
    for (int sequenceNum = 0; sequenceNum < max; sequenceNum++)
    {
        message[0] = sequenceNum;
        sock.sendTo((char *)message, MSGSIZE);

        // Start the timer for ack reception
        timer.start();

        // Wait while there is no data to receive and wait for the timer to expire
        // to resend the frame
        while (sock.pollRecvFrom() < 1)
        {
            // Resend the frame, restart the ack timer, and increment the retransmission count
            if (timer.lap() > TIMEOUT)
            {
                sock.sendTo((char *)message, MSGSIZE);
                timer.start();
                retransmitted++;
            }
        }

        // Exit from the above while loop means the ack has been received.
        // Since the server checks for the correct order of sent frames
        // there is little need for the same check on the client side
        sock.recvFrom((char *)message, MSGSIZE);
    }
    return retransmitted;
}

// Repeats receiving message[] and sending an acknowledgment at a server side
// max (=20,000) times using the sock object. This test may not reach the peak
// performance of underlying network. This is because the client must wait for an
// acknowledgment every time it sends out a new message.
void serverReliable( UdpSocket &sock, const int max, int message[] )
{
    // Loop until all 20000 frames have been received
    for (int sequenceNum = 0; sequenceNum < max; sequenceNum++)
    {
        // Execute reception first then compare if the sequence is out of order
        do
        {
            sock.recvFrom((char *)message, MSGSIZE);

            // Send ack if frame is correct (i.e. in order)
            if (message[0] == sequenceNum)
            {
                sock.ackTo((char *)&sequenceNum, sizeof(sequenceNum));
            }
        }
        while (message[0] != sequenceNum);
    }
}

// Repeats sending message[] and receiving an acknowledgment at a client side
// max (=20,000) times using the sock object. As described above, the client can
// continuously send a new message[] as incrementing its sequence number as far as
// the number of in-transit messages, (i.e., # of unacknowledged messages) is less
// than windowSize. That number should be decremented every time the client receives
// an acknowledgment. If # of unacknowledged messages reaches windowSize, the client
// should start a timer. If a timeout (= 1500usec) has happened, it must send the message
// with the minimum sequence number among those which have not yet been acknowledged.
// The function must count the number of messages retransmitted and return it to the
// main function as its return value.
int clientSlidingWindow( UdpSocket &sock, const int max, int message[], int windowSize )
{
    int retransmitted = 0;
    int nextFrame = 0;
    int lastFrame = 0;
    int lastAckRec = INT16_MIN;
    Timer timer;
    
    // Loop until all frames have been sent to the server and all ack sent back
    while ((nextFrame < max) || (lastFrame < max))
    {
        // Keep sending frames as long as the transit frames count is less than
        // the window size. As the frames are sent the window size is decresed
        // and by receiving ack from the server the window size is increased
        if (((lastFrame + windowSize) > nextFrame) && (nextFrame < max))
        {
            message[0] = nextFrame;
            sock.sendTo((char *)message, MSGSIZE);
            nextFrame++;
        }
        // Miniscule delay so the read is not instantaneous
        usleep(1);
        
        // If there is data to receive, save the ack and increment lastFrame,
        // otherwise start the timeout timer
        if (sock.pollRecvFrom() > 0)
        {
            sock.recvFrom((char *)&lastAckRec, sizeof(lastAckRec));
            if (lastAckRec == lastFrame)
            {
                lastFrame++;
            }
        }
        else
        {
            timer.start();
            // Listen for an ack from the server
            while (sock.pollRecvFrom() < 1)
            {
                if (timer.lap() > TIMEOUT)
                {
                    retransmitted += (nextFrame - lastFrame);
                    
                    // Checks to see if the frame is late, otherwise it is lost
                    if ((lastAckRec >= lastFrame) && (lastAckRec <= nextFrame))
                    {
                        lastFrame = (lastAckRec + 1);
                    }
                    else
                    {
                        nextFrame = lastFrame;
                    }
                    break;
                }
            }
        }
    }
    return retransmitted;
}

// Repeats receiving message[] and sending an acknowledgment at a server side max (=20,000)
// times using the sock object. Every time the server receives a new message[], it must
// memorizes this message's sequence number in its array and returns a cumulative acknowledgment
void serverEarlyRetrans( UdpSocket &sock, const int max, int message[], int windowSize )
{
    int lastFrameRec = 0;
    int lastAckRec = 0;
    int lastFrame = INT16_MIN;
    int sequenceNum;
    vector<int> recFramesArray(windowSize, -1); // Array for cumulative acknowledgement

    // Loop until the last received frame number reaches max (i.e. 20000)
    while(lastFrameRec < (max - 1))
    {
        // Put the server to sleep until it has data to receive
        while( sock.pollRecvFrom() < 1 )
        {
            usleep(1);
        }
        sock.recvFrom( ( char * ) message, MSGSIZE );
        lastFrameRec = *message;
        
        // Extract the message sequence number
        sequenceNum = (lastFrameRec % windowSize);
        
        // If the message sequence number is outside the window size drop frame.
        // Else if the frame is next in sequence to be acknowledged (i.e. first unacknowledged frame).
        // Else the message sequence number is inside the window size and store in the array
        if ((sequenceNum >= windowSize) || (sequenceNum < lastAckRec))
        {
            sock.ackTo((char *) &lastFrame, sizeof(int));
            continue;
        }
        else if(sequenceNum == lastAckRec)
        {
            recFramesArray[sequenceNum] = lastFrameRec;
            lastFrame = lastFrameRec;
            
            // Traverse until you reach an unacknowledged frame
            while(recFramesArray[lastAckRec] > -1)
            {
                // Update the last frame received
                lastFrame = recFramesArray[lastAckRec];
                
                // Update that the you have received the frame and free up the sequence number
                recFramesArray[lastAckRec] = -1;
                
                // Increase (shift) the window
                lastAckRec++;
                lastAckRec = (lastAckRec % windowSize);
                
                // If we have received all packets in order so far, or hit a sequence of
                // unacknowledged frames, send ack for last frame
                if (recFramesArray[lastAckRec] == -1)
                {
                    sock.ackTo((char *) &lastFrame, sizeof(int));
                }
            }
        }
        else
        {
            recFramesArray[sequenceNum] = lastFrameRec;
            sock.ackTo((char *) &lastFrame, sizeof(int));
        }
    }
}

