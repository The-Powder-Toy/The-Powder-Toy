#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

class SearchModel;

class SearchView
{
public:
	void NotifySaveListChanged(SearchModel * sender);
    SearchView();
};

#endif // SEARCHVIEW_H
