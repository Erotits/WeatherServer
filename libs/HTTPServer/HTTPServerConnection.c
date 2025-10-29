#include "HTTPServerConnection.h"
#include <stdlib.h>
#include <stdio.h>

//-----------------Internal Functions-----------------

void HTTPServerConnection_TaskWork(void* _Context, uint64_t _MonTime);

//----------------------------------------------------

int HTTPServerConnection_Initiate(HTTPServerConnection* _Connection, int _FD)
{
	TCPClient_Initiate(&_Connection->tcpClient, _FD);
	
	_Connection->task = smw_createTask(_Connection, HTTPServerConnection_TaskWork);

	return 0;
}

int HTTPServerConnection_InitiatePtr(int _FD, HTTPServerConnection** _ConnectionPtr)
{
	if(_ConnectionPtr == NULL)
		return -1;

	HTTPServerConnection* _Connection = (HTTPServerConnection*)malloc(sizeof(HTTPServerConnection));
	if(_Connection == NULL)
		return -2;

	int result = HTTPServerConnection_Initiate(_Connection, _FD);
	if(result != 0)
	{
		free(_Connection);
		return result;
	}

	*(_ConnectionPtr) = _Connection;

	return 0;
}

void HTTPServerConnection_SetCallback(HTTPServerConnection* _Connection, void* _Context, HTTPServerConnection_OnRequest _OnRequest)
{
	_Connection->context = _Context;
	_Connection->onRequest = _OnRequest;
}

void HTTPServerConnection_TaskWork(void *_Context, uint64_t _MonTime)
{
    HTTPServerConnection *_Connection = (HTTPServerConnection *)_Context;

    if (_Connection == NULL)
        return;

    _Connection->method = NULL;
    _Connection->url = NULL;

    char buffer[1024];

    int bytesRead = TCPClient_Read(&_Connection->tcpClient, (uint8_t *)buffer, sizeof(buffer));
    printf("%i\n", bytesRead);
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        char *ptr = &buffer[0];
        char *space = strchr(ptr, ' ');
        if (space != NULL)
        {
            int method_length = space - ptr;

            _Connection->method = strndup(ptr, method_length);
            if (strcmp(_Connection->method, "GET") == 0)
                printf("%s\n", _Connection->method);
        }

        char *host = strstr(ptr, "Host: ");
        if (host != NULL)
        {
            host += strlen("Host: ");

            char *eol = strstr(host, "\r\n");

            if (eol != NULL)
            {
                int url_length = eol - host;

                _Connection->url = strndup(host, url_length);
                printf("%s\n", _Connection->url);
            }
        }
    }


        if (_Connection->method != NULL && _Connection->url != NULL && (strcmp(_Connection->method, "GET") == 0))
    {
        printf("Method: %s\nURL: %s\n", _Connection->method, _Connection->url);
        
        if(strstr(buffer, "\r\n\r\n"))
        _Connection->onRequest(_Connection->context);
    }
    
}




/* Second solution
void HTTPServerConnection_TaskWork(void* _Context, uint64_t _MonTime)
{
	HTTPServerConnection* _Connection = (HTTPServerConnection*)_Context;

	if(_Connection == NULL)
	{
		return;
	}
	char buffer[1024];

	int bytesRead = TCPClient_Read(&_Connection->tcpClient, (uint8_t *)buffer, sizeof(buffer));

	if(bytesRead > 0)
	{
		
		buffer[bytesRead] = '\0';
		char* ptr = &buffer[0];
		
		if(ptr == NULL)
		{
			return;
		}
		char* line = strndup(ptr, *strchr(buffer, ' '));
		
		char* pch = strtok(line, " ");
		_Connection->method = strdup(pch);
		
		pch = strtok(NULL, "\r\n");
		_Connection->url =  strdup(pch);
	}
	else
	{
		return;
	}	
	if (_Connection->method != NULL && _Connection->url != NULL && (strcmp(_Connection->method, "GET") == 0))
    {
        printf("Method: %s\nURL: %s\n", _Connection->method, _Connection->url);
        
        if(strstr(buffer, "\r\n\r\n"))
        	_Connection->onRequest(_Connection->context);
    }
}
*/

void HTTPServerConnection_Dispose(HTTPServerConnection* _Connection)
{
	TCPClient_Dispose(&_Connection->tcpClient);
	smw_destroyTask(_Connection->task);
}

void HTTPServerConnection_DisposePtr(HTTPServerConnection** _ConnectionPtr)
{
	if(_ConnectionPtr == NULL || *(_ConnectionPtr) == NULL)
		return;

	HTTPServerConnection_Dispose(*(_ConnectionPtr));
	free(*(_ConnectionPtr));
	*(_ConnectionPtr) = NULL;
}
