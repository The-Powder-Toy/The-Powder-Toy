#ifndef APIRESULTPARSER_H
#define APIRESULTPARSER_H

class APIResultParser
{
public:
	//Process the raw API response into a result object to be returned to the requester
	virtual void * ProcessResponse(unsigned char * data, int dataLength) { return 0; }

	//A method to clean up the result of ProcessResponse in the event of a callback failure in APIRequest/RequestBroker
	virtual void Cleanup(void * objectPtr) { }

	virtual ~APIResultParser() { }
};

#endif
