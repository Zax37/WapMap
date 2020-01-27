#include "wTextBox.h"

#include "guichan/basiccontainer.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseinput.hpp"

#ifdef WAP_MAP

#include "../../WapMap/globals.h"

#endif

namespace SHR {
    TextBox::TextBox() {
        mCaretColumn = 0;
        mCaretRow = 0;
        mEditable = true;
        mOpaque = true;

        mMinX = mMinY = -1;

        setText("");

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        adjustSize();
    }

    TextBox::TextBox(const std::string &text) {
        mCaretColumn = 0;
        mCaretRow = 0;
        mEditable = true;
        mOpaque = true;

        mMinX = mMinY = -1;

        setText(text);

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        adjustSize();
    }

    void TextBox::setText(const std::string &text) {
        mCaretColumn = 0;
        mCaretRow = 0;

        mTextRows.clear();

        std::string::size_type pos, lastPos = 0;
        int length;
        do {
            pos = text.find("\n", lastPos);

            if (pos != std::string::npos) {
                length = pos - lastPos;
            } else {
                length = text.size() - lastPos;
            }
            std::string sub = text.substr(lastPos, length);
            mTextRows.push_back(sub);
            lastPos = pos + 1;

        } while (pos != std::string::npos);

        adjustSize();
    }

    void TextBox::draw(Graphics *graphics) {
        unsigned int i;

        if (mOpaque) {
            graphics->setColor(isFocused() ? 0x434343 : 0x3c3c3c);
            graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
        }

        if (isFocused() && isEditable()) {
            drawCaret(graphics, getFont()->getWidth(mTextRows[mCaretRow].substr(0, mCaretColumn)),
                      mCaretRow * getFont()->getHeight());
        }

        graphics->setColor(getForegroundColor());
        graphics->setFont(getFont());

        for (i = 0; i < mTextRows.size(); i++) {
            // Move the text one pixel so we can have a caret before a letter.
            graphics->drawText(mTextRows[i], 1, i * getFont()->getHeight());
        }
    }

    void TextBox::drawCaret(Graphics *graphics, int x, int y) {
        graphics->setColor(getForegroundColor());
        graphics->drawLine(x, getFont()->getHeight() + y, x, y);
    }

    void TextBox::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mCaretRow = mouseEvent.getY() / getFont()->getHeight();

            if (mCaretRow >= (int) mTextRows.size()) {
                mCaretRow = mTextRows.size() - 1;
            }

            mCaretColumn = getFont()->getStringIndexAt(mTextRows[mCaretRow], mouseEvent.getX());
        }
    }

    void TextBox::mouseDragged(MouseEvent &mouseEvent) {
        mouseEvent.consume();
    }

    void TextBox::keyPressed(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();

        if (key.getValue() == Key::LEFT) {
            --mCaretColumn;
            if (mCaretColumn < 0) {
                --mCaretRow;

                if (mCaretRow < 0) {
                    mCaretRow = 0;
                    mCaretColumn = 0;
                } else {
                    mCaretColumn = mTextRows[mCaretRow].size();
                }
            }
        } else if (key.getValue() == Key::RIGHT) {
            ++mCaretColumn;
            if (mCaretColumn > (int) mTextRows[mCaretRow].size()) {
                ++mCaretRow;

                if (mCaretRow >= (int) mTextRows.size()) {
                    mCaretRow = mTextRows.size() - 1;
                    if (mCaretRow < 0) {
                        mCaretRow = 0;
                    }

                    mCaretColumn = mTextRows[mCaretRow].size();
                } else {
                    mCaretColumn = 0;
                }
            }
        } else if (key.getValue() == Key::DOWN) {
            setCaretRow(mCaretRow + 1);
        } else if (key.getValue() == Key::UP) {
            setCaretRow(mCaretRow - 1);
        } else if (key.getValue() == Key::HOME) {
            mCaretColumn = 0;
        } else if (key.getValue() == Key::END) {
            mCaretColumn = mTextRows[mCaretRow].size();
        } else if (key.getValue() == Key::ENTER && mEditable) {
            mTextRows.insert(mTextRows.begin() + mCaretRow + 1,
                             mTextRows[mCaretRow].substr(mCaretColumn, mTextRows[mCaretRow].size() - mCaretColumn));
            mTextRows[mCaretRow].resize(mCaretColumn);
            ++mCaretRow;
            mCaretColumn = 0;
        } else if (key.getValue() == Key::BACKSPACE
                   && mCaretColumn != 0
                   && mEditable) {
            mTextRows[mCaretRow].erase(mCaretColumn - 1, 1);
            --mCaretColumn;
            distributeActionEvent();
        } else if (key.getValue() == Key::BACKSPACE
                   && mCaretColumn == 0
                   && mCaretRow != 0
                   && mEditable) {
            mCaretColumn = mTextRows[mCaretRow - 1].size();
            mTextRows[mCaretRow - 1] += mTextRows[mCaretRow];
            mTextRows.erase(mTextRows.begin() + mCaretRow);
            --mCaretRow;
            distributeActionEvent();
        } else if (key.getValue() == Key::DELETE
                   && mCaretColumn < (int) mTextRows[mCaretRow].size()
                   && mEditable) {
            mTextRows[mCaretRow].erase(mCaretColumn, 1);
            distributeActionEvent();
        } else if (key.getValue() == Key::DELETE
                   && mCaretColumn == (int) mTextRows[mCaretRow].size()
                   && mCaretRow < ((int) mTextRows.size() - 1)
                   && mEditable) {
            mTextRows[mCaretRow] += mTextRows[mCaretRow + 1];
            mTextRows.erase(mTextRows.begin() + mCaretRow + 1);
            distributeActionEvent();
        } else if (key.getValue() == Key::PAGE_UP) {
            Widget *par = getParent();

            if (par != NULL) {
                int rowsPerPage = par->getChildrenArea().height / getFont()->getHeight();
                mCaretRow -= rowsPerPage;

                if (mCaretRow < 0) {
                    mCaretRow = 0;
                }
            }
        } else if (key.getValue() == Key::PAGE_DOWN) {
            Widget *par = getParent();

            if (par != NULL) {
                int rowsPerPage = par->getChildrenArea().height / getFont()->getHeight();
                mCaretRow += rowsPerPage;

                if (mCaretRow >= (int) mTextRows.size()) {
                    mCaretRow = mTextRows.size() - 1;
                }
            }
        } else if (key.getValue() == Key::TAB
                   && mEditable) {
            mTextRows[mCaretRow].insert(mCaretColumn, std::string("    "));
            mCaretColumn += 4;
            distributeActionEvent();
        } else if (key.isCharacter()
                   && mEditable) {
            mTextRows[mCaretRow].insert(mCaretColumn, std::string(1, (char) key.getValue()));
            ++mCaretColumn;
            distributeActionEvent();
        }

        adjustSize();
        scrollToCaret();

        keyEvent.consume();
    }

    void TextBox::adjustSize() {
        unsigned int i;
        int width = 0;
        for (i = 0; i < mTextRows.size(); ++i) {
            int w = getFont()->getWidth(mTextRows[i]);
            if (width < w) {
                width = w;
            }
        }

        if (width < mMinX)
            setWidth(mMinX);
        else
            setWidth(width + 1);

        if (getFont()->getHeight() * mTextRows.size() < mMinY)
            setHeight(mMinY);
        else
            setHeight(getFont()->getHeight() * mTextRows.size());
    }

    void TextBox::setCaretPosition(unsigned int position) {
        int row;

        for (row = 0; row < (int) mTextRows.size(); row++) {
            if (position <= mTextRows[row].size()) {
                mCaretRow = row;
                mCaretColumn = position;
                return; // we are done
            } else {
                position--;
            }
        }

        // position beyond end of text
        mCaretRow = mTextRows.size() - 1;
        mCaretColumn = mTextRows[mCaretRow].size();
    }

    unsigned int TextBox::getCaretPosition() const {
        int pos = 0, row;

        for (row = 0; row < mCaretRow; row++) {
            pos += mTextRows[row].size();
        }

        return pos + mCaretColumn;
    }

    void TextBox::setCaretRowColumn(int row, int column) {
        setCaretRow(row);
        setCaretColumn(column);
    }

    void TextBox::setCaretRow(int row) {
        mCaretRow = row;

        if (mCaretRow >= (int) mTextRows.size()) {
            mCaretRow = mTextRows.size() - 1;
        }

        if (mCaretRow < 0) {
            mCaretRow = 0;
        }

        setCaretColumn(mCaretColumn);
    }

    unsigned int TextBox::getCaretRow() const {
        return mCaretRow;
    }

    void TextBox::setCaretColumn(int column) {
        mCaretColumn = column;

        if (mCaretColumn > (int) mTextRows[mCaretRow].size()) {
            mCaretColumn = mTextRows[mCaretRow].size();
        }

        if (mCaretColumn < 0) {
            mCaretColumn = 0;
        }
    }

    unsigned int TextBox::getCaretColumn() const {
        return mCaretColumn;
    }

    const std::string &TextBox::getTextRow(int row) const {
        return mTextRows[row];
    }

    void TextBox::setTextRow(int row, const std::string &text) {
        mTextRows[row] = text;

        if (mCaretRow == row) {
            setCaretColumn(mCaretColumn);
        }

        adjustSize();
    }

    unsigned int TextBox::getNumberOfRows() const {
        return mTextRows.size();
    }

    std::string TextBox::getText() const {
        if (mTextRows.size() == 0) {
            return std::string("");
        }

        int i;
        std::string text;

        for (i = 0; i < (int) mTextRows.size() - 1; ++i) {
            text = text + mTextRows[i] + "\n";
        }

        text = text + mTextRows[i];

        return text;
    }

    void TextBox::fontChanged() {
        adjustSize();
    }

    void TextBox::scrollToCaret() {
        gcn::Rectangle scroll;
        scroll.x = getFont()->getWidth(mTextRows[mCaretRow].substr(0, mCaretColumn));
        scroll.y = getFont()->getHeight() * mCaretRow;
        scroll.width = getFont()->getWidth(" ");
        scroll.height = getFont()->getHeight() + 2; // add 2 for some extra space

        showPart(scroll);
    }

    void TextBox::setEditable(bool editable) {
        mEditable = editable;
    }

    bool TextBox::isEditable() const {
        return mEditable;
    }

    void TextBox::addRow(const std::string row) {
        mTextRows.push_back(row);
        adjustSize();
    }

    bool TextBox::isOpaque() {
        return mOpaque;
    }

    void TextBox::setOpaque(bool opaque) {
        mOpaque = opaque;
    }
}
