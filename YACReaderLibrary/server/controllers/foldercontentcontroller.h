#ifndef FOLDERCONTENTCONTROLLER_H
#define FOLDERCONTENTCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

class FolderContentController : public HttpRequestHandler {
	Q_OBJECT
    Q_DISABLE_COPY(FolderContentController);
public:
	/** Constructor */
    FolderContentController();

	/** Generates the response */
	void service(HttpRequest& request, HttpResponse& response);

private:
    void serviceContent(const int &library, const qulonglong &folderId, HttpResponse &response);
};

#endif // FOLDERCONTENTCONTROLLER_H
