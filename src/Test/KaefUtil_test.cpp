// KaefUtil.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <vector>
#include <KaefUtil/XML_Writer.h>
#include <KaefUtil/InterfaceIterator.h>
#include "DateTimeTest.h"

using namespace KaefUtil;
using namespace DateTimeTest;


int main() {
	constexpr DateTime D1{ 0 };
	constexpr int y = D1.year(),
		m = D1.month(),
		d = D1.day();
	constexpr auto offs0 = DateTime::dayOffset(1, 1, 1);

	using std::cout;
	cout << "\n[Testing iteration] ...";
	DateTimeTestIterate();
	cout << " [done!]";

	cout << "\n[Testing day offsets] ...";
	DateTimeTestDayOffset();
	cout << " [done!]";
/*#define RELAX(...) __VA_ARGS__
#define CONTENT(a,...) __VA_ARGS__
#define INPUT(FLD, GRP, GFLD) \
	FLD(int, a) \
	GRP(Gru, \
		RELAX(GFLD(bool,   name, b)) \
		GFLD(size_t, g, c) \
	)

#define FLD1(type,name) type name;
#define GRP1(name,...) struct { __VA_ARGS__ } name;
#define GFLD1(type,space,name) type name;
	INPUT(FLD1, GRP1, GFLD1)

#define FLD2(type, name) name,
#define GFLD2(type,space,name) name##_##space,

		enum { INPUT(FLD2, CONTENT, GFLD2) };*/
	

/* RELAX(SKIPF(ignoreme X,D)) */

	/*XML_Writer W{ std::cout, false, "\r\n", "  " };
	W.setStyle(false, XML_Writer::HTML_FORM);
	W.s("!DOCTYPE", {"html"}).f("html")("head").s("meta", {{"charset", "UTF-8"}}).s("meta")
		.i("title").c("Sample document")()().f("body");
	W.i("h1").c("A document ").i("i").c("(sample)")()();
	W("table")("tr").i("th").c("Column 1")().i("th").c("Column 2")()()();

	W.flush();*/
	std::cout << std::endl << std::endl;
}
