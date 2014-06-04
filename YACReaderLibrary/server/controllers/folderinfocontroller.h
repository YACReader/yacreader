#ifndef FOLDERINFOCONTROLLER_H
#define FOLDERINFOCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FolderInfoController : public HttpRequestHandler {
	Q_OBJECT
		Q_DISABLE_COPY(FolderInfoController);
public:

	/** Constructor */
	FolderInfoController();

	/** Generates the response */
	void service(HttpRequest& request, HttpResponse& response);
};

#endif // FOLDERINFOCONTROLLER_H
