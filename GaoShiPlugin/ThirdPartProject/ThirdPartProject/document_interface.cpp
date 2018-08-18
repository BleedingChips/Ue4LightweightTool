#include "document_interface.h"
#include "tool/document.h"
#include "assert.h"
PO::Doc::Format translate(Doc::Format format)
{
	switch (format)
	{
	case Doc::Format::NOT_UNICIDE:
		return PO::Doc::Format::NOT_UNICODE;
	case Doc::Format::UTF8_WITH_BOM:
		return PO::Doc::Format::UTF8_WITH_BOM;
	case Doc::Format::UTF8:
		return PO::Doc::Format::UTF8;
	}
	return PO::Doc::Format::UTF8;
}

Doc::Format translate(PO::Doc::Format format)
{
	switch (format)
	{
	case PO::Doc::Format::UTF8:
		return Doc::Format::UTF8;
	case PO::Doc::Format::UTF8_WITH_BOM:
		return Doc::Format::UTF8_WITH_BOM;
	case PO::Doc::Format::NOT_UNICODE:
		return Doc::Format::NOT_UNICIDE;
	default:
		assert(false);
		break;
	}
	return Doc::Format::UTF8;
}


namespace Doc
{
	writer::writer(const wchar_t* path, Format format) : implement(new PO::Doc::writer<wchar_t>{ path, ::translate(format) }) {}
	writer::~writer() { delete reinterpret_cast<PO::Doc::writer<wchar_t>*>(implement); }
	void writer::write(const wchar_t* string, size_t count) { reinterpret_cast<PO::Doc::writer<wchar_t>*>(implement)->write(string, count); }
	void writer::write(const wchar_t* string) { reinterpret_cast<PO::Doc::writer<wchar_t>*>(implement)->write(string); }
	bool writer::is_open() const noexcept { return reinterpret_cast<PO::Doc::writer<wchar_t>*>(implement)->is_open(); }
}