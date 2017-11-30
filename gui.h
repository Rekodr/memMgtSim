#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCore>
#include <QtGui>
#include <qfiledialog.h>

#include "ui_gui.h"
#include "memPjr.h"
#include <string>

class gui : public QMainWindow
{
	Q_OBJECT

public:
	gui(char* filename, QWidget *parent = Q_NULLPTR);

private:
	Json::Value ms;
	Ui::guiClass ui;
	memProject::File* f = nullptr;
	memProject::Mem mem;
	int currPosition{ -1 };
	unsigned int numProcs{ 0 };
	void addRoot(Json::Value& value);
	void addChildren(QTreeWidgetItem* parent, Json::Value& value);
	bool update(std::string& js, bool nextFault);
	bool update(Json::Value& root);
	void updateFrame(Json::Value& value);
	std::string openFile();

	private slots:
		void nextQ();
		void completeQ();
		void nextQFault();
		void prevQ();
		void resetQ();
		void fChangeQ();

};
