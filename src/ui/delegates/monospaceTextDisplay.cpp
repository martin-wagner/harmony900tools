// SPDX-License-Identifier: LGPL-2.1-or-later

#include "monospaceTextDisplay.h"

#include <QFontDatabase>
#include <QPainter>

namespace delegates
{

MonospaceTextDisplay::MonospaceTextDisplay(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

MonospaceTextDisplay::~MonospaceTextDisplay() = default;

void MonospaceTextDisplay::paint(QPainter *painter,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  painter->save();

  if ((option.state & QStyle::State_Selected) != 0) {
    painter->fillRect(option.rect, option.palette.highlight());
    painter->setPen(option.palette.highlightedText().color());
  } else {
    painter->setPen(option.palette.text().color());
  }

  const QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  painter->setFont(monoFont);

  const QString text = index.data(Qt::DisplayRole).toString();
  painter->drawText(option.rect, Qt::TextWordWrap, text);

  painter->restore();
}

QSize MonospaceTextDisplay::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
  const QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  const QFontMetrics metrics(monoFont);

  const QString text = index.data(Qt::DisplayRole).toString();
  const QRect boundingRect = metrics.boundingRect(option.rect, Qt::TextWordWrap,
      text);

  return boundingRect.size();
}

QWidget* MonospaceTextDisplay::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  //display only, no editing
  return nullptr;
}

}
