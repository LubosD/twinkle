#include "textbrowsernoautolink.h"

TextBrowserNoAutoLink::TextBrowserNoAutoLink (QWidget* parent) :
	QTextBrowser(parent)
{
}

#if QT_VERSION >= 0x050000
void TextBrowserNoAutoLink::setSource(const QUrl & name)
{
}
#else

void TextBrowserNoAutoLink::setSource ( const QString & name )
{
}

#endif
