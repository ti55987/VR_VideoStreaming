Streaming Feature Integration Status

Editted by Intern : Ti-Fen Pan
Date: 8/21/2015

Introduction:
	1. libdash is an OO interface for MPD 
	2. libdash relies on mainly four libraries: libcurl, libxml, iconv, zlib
	3. Test is a VS2013 project for downloading segments demo
	4. Please put the Streaming directory with ffmpeg, ffts ,portaudio and OculusSDK
	   Follow the structure as below:
		--Streaming
			--3dParty
			--curl_mingw64
			--libdash
			--LibHVR
			--LibOVR-0.4.4
			--PresencePlayer
			--Test
			--README
		--ffmpeg
	   	--ffts
	   	--portaudio
	   	--OculusSDK

	5. PresencePlayer folder contains:
	   -- original codes
	   -- some libdash player sample codes in libdashframework folder
	   -- my defined class StreamingProvider

	6. To enable the code I write for streaming in PresencePlayer, please add #define STREAMING_VIDEO

Done:
	1. Found and import the 64 bit library for libcurl to PresencePlayer
	2. Finished downloading semgments 
	3. Packaged the downloading feature as a StreamingProvider
	4. Demo: running Test VS2013 project 
	5. Integrated the output of StreamingPrvodier to the function in map4provider::load in PresencePlayer

Undone:
	1. Re-build three libraries for importing to PresencePlayer
	   -- libxml 
	   -- iconv
	   -- zlib
	   These three pre-built libraries in libdash are all built in 32 bit.

	2. In map4provider::run in PresencePlayer, Line 441 the loading part should be modified for downloading more segments.
