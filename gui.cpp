#include "stdafx.h"
#include "gui.h"
#include <iostream>
#include <cstdio>
#include <string>

gui::gui(char* fileName, QWidget *parent)
	: QMainWindow(parent )
{

	
	ui.setupUi(this);
	openFile();

	ui.frameTable->setColumnCount(4);

	QStringList headers1;
	

	headers1 << "Frame ID" << "Owner PID" << "Page ID" << "Second Chance";
	ui.frameTable->setHorizontalHeaderLabels(headers1);
	ui.frameTable->setShowGrid(true);

	ui.pcbTree->setColumnCount(4);
	QStringList headers2;
	headers2 << "Owner PID" << "Table size" << "Number of Ref" << "Faulted ref";
	ui.pcbTree->setHeaderLabels(headers2);
	ui.pcbTree->header()->setDefaultAlignment(Qt::AlignCenter);
	ui.pcbTree->setAnimated(true);
	

	ui.nextQ->setIcon(QIcon("Resources/next"));
	ui.completeQ->setIcon(QIcon("Resources/Fast_Forward"));
	ui.prevQ->setIcon(QIcon("Resources/previous"));
	ui.runQ->setIcon(QIcon("Resources/nextFault"));
	ui.resetQ->setIcon(QIcon("Resources/reset"));
	ui.fChange->setIcon(QIcon("Resources/file"));

	connect(ui.nextQ, SIGNAL(clicked()), this, SLOT(nextQ()));
	connect(ui.completeQ, SIGNAL(clicked()), this, SLOT(completeQ()));
	connect(ui.runQ, SIGNAL(clicked()), this, SLOT(nextQFault()));
	connect(ui.prevQ, SIGNAL(clicked()), this, SLOT(prevQ()));
	connect(ui.resetQ, SIGNAL(clicked()), this, SLOT(resetQ()));
	connect(ui.fChange, SIGNAL(clicked()), this, SLOT(fChangeQ()));

}

void gui::nextQ()
{
	try {
		auto Q = f->next();
		auto js = mem.search_PCB(Q);
		update(js, false);
	}
	catch (std::exception e) {
		
	}
}

void gui::completeQ()
{
	std::string js;
	currPosition--;
	while (1) {
		try {
			auto Q = f->next();
			js = mem.search_PCB(Q);
			currPosition++;
		}
		catch (std::exception e) {
			break;
		}
	}
	update(js, false);
}

void gui::nextQFault()
{
	while (1) {
		try {
			auto Q = f->next();
			auto js = mem.search_PCB(Q);
			auto fault = update(js, true);
			if (fault)
				break;
		}
		catch (std::exception e) {
			break;
		}
	}
}

void gui::prevQ()
{
	mem.loadJson(ms, currPosition - 1);
	currPosition--;
	update(ms);
	f->gotTo();
}

void gui::resetQ()
{
	mem.clear();
	f->rewind();
	ui.frameTable->clearContents();
	ui.pcbTree->clear();
	currPosition = -1;
}

void gui::fChangeQ()
{
	mem.clear();
	openFile();
	currPosition = -1;
	ui.frameTable->clearContents();
	ui.pcbTree->clear();
}

void gui::addRoot(Json::Value& value)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(ui.pcbTree);

	auto id = value["id"].asInt(); 
	auto pgTable = value["pgTable"];
	auto pageTsize = pgTable["pageTableSize"].asInt();
	auto numFaults = pgTable["pageFaults"].asInt();
	auto numOfRef = pgTable["numOfRef"].asInt();

	item->setText(0, QString::number(id));
	item->setText(1, QString::number(pageTsize));
	item->setText(2, QString::number(numOfRef));
	item->setText(3, QString::number(numFaults));
	for (auto i = 0; i < ui.pcbTree->columnCount(); i++) {
		item->setBackgroundColor(i, QColor::QColor(Qt::lightGray));
	}

	Json::Value pages = pgTable["pages"];
	for (auto i = 0; i < ui.pcbTree->columnCount(); i++) {
		item->setTextAlignment(i, Qt::AlignCenter);
	}

	addChildren(item, pages);
}

void gui::addChildren(QTreeWidgetItem * parent, Json::Value& value)
{
	
	for (auto& page : value) {

		QTreeWidgetItem* item = new QTreeWidgetItem();
		auto pageId = page["pageId"].asInt();
		auto frameId = page["frameId"].asInt();
		auto resident = page["resident"].asBool();
		auto referenced = page["referenced"].asBool();
		auto hasChanged = page["hasChanged"].asBool();

		item->setText(0, "Pg#: " + QString::number(pageId));
		if (hasChanged) {
			for (auto i = 0; i < ui.pcbTree->columnCount(); i++) {
				item->setBackgroundColor(i, QColor::QColor(Qt::yellow));
				parent->setBackgroundColor(i, QColor::QColor(Qt::darkCyan));
				parent->setExpanded(true);
				
			}
		}
		
		if (resident) {
			item->setText(1, "F#: " + QString::number(frameId));
			item->setIcon(0, QIcon("Resources/in_mem.ico"));
		}
		else {
			item->setText(1, "F#: N/A");
			item->setIcon(0, QIcon("Resources/not_in_mem"));
		}

		for (auto i = 0; i < ui.pcbTree->columnCount(); i++) {
			item->setTextAlignment(i, Qt::AlignCenter);
		}

		parent->addChild(item);
	}
	
}

bool gui::update(std::string& js, bool nextFault)
{

	
	Json::Value root;
	Json::Reader reader;
	reader.parse(js, root);
	ms = root; 
	auto snapshots = root["snapshots"];

	Json::Value snap;

	for (auto snapShot : snapshots) {
		if (snapShot["atime"].asInt() == (currPosition + 1)) {
			snap = snapShot;
			break;
		}
	}
	currPosition = snap["atime"].asInt();
	auto faulted = snap["faulted"].asBool();

	if (faulted == false && nextFault == true)
		return false;

	
	numProcs = snap["num_procs"].asUInt64();
	auto pcbTable = snap["pcbTable"];

	ui.pcbTree->clear();
	for (auto& pcb : pcbTable) {

		addRoot(pcb);
	}

	auto frameTable = snap["frameTable"];
	updateFrame(frameTable);
	return true;
}

bool gui::update(Json::Value& root)
{
	auto snapshots = root["snapshots"];
	Json::Value snap;

	for (auto snapShot : snapshots) {
		if (snapShot["atime"].asInt() == (currPosition)) {
			snap = snapShot;
			break;
		}
	}
	auto faulted = snap["faulted"].asBool();

	

	currPosition = snap["atime"].asInt();
	numProcs = snap["num_procs"].asUInt64();
	auto pcbTable = snap["pcbTable"];

	ui.pcbTree->clear();
	for (auto& pcb : pcbTable) {

		addRoot(pcb);
	}

	auto frameTable = snap["frameTable"];
	updateFrame(frameTable);
	return true;
}


void gui::updateFrame(Json::Value& value)
{
	ui.frameTable->clearContents();
	auto n = value.size();
	ui.frameTable->setRowCount(n);
	ui.frameTable->verticalHeader()->setVisible(false);

	for (auto& frame : value) {
		auto id = frame["id"].asInt();
		auto empt = frame["free"].asBool();
		auto isClockHand = frame["isClockHand"].asBool();
		auto pid = -1;
		auto pgId = -1;
		
		if (empt == false) {
			pid = frame["pid"].asInt();
			pgId = frame["pageId"].asInt();
		}
		auto refBit = frame["referenced"].asBool();
		auto  hasChanged = frame["hasChanged"].asBool();

		QIcon ss = QIcon("Resources/cry");
		if (refBit)
			 ss = QIcon("Resources/happy");

		ui.frameTable->setItem(id, 0, new QTableWidgetItem(QString::number(id)));
		ui.frameTable->setItem(id, 1, new QTableWidgetItem(QString::number(pid)));
		ui.frameTable->setItem(id, 2, new QTableWidgetItem(QString::number(pgId)));

		ui.frameTable->setItem(id, 3, new QTableWidgetItem());

		ui.frameTable->item(id, 3)->setIcon(ss);

		

		if (isClockHand) {
			ui.frameTable->item(id, 0)->setIcon(QIcon("Resources/pointer"));
		}
		if (hasChanged) {

			for (auto i = 0; i < ui.frameTable->columnCount(); i++) {
				ui.frameTable->item(id, i)->setBackgroundColor(QColor::QColor(Qt::yellow));
			}
		}
		else
		{
			for (auto i = 0; i < ui.frameTable->columnCount(); i++) {
				ui.frameTable->item(id, i)->setBackgroundColor(QColor::QColor(Qt::darkGray));
			}

		}

		for (auto i = 0; i < ui.frameTable->columnCount(); i++) {
			ui.frameTable->item(id, i)->setTextAlignment(Qt::AlignCenter);
		}
	}
}

std::string gui::openFile()
{
	std::string fname;
	
	QString filepath = "";
	while(filepath.isEmpty())
		filepath = QFileDialog::getOpenFileName(this, tr("Open File"), ".", 
					"Data Files (*.data);; Text Files (*.txt) ");

	fname = filepath.toStdString();
	if (f == nullptr)
		f = new memProject::File(fname);
	else
		f->changeFile(fname);

	return fname;
}
