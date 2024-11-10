/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <config-test.h>
#include <gtest/gtest.h>

#include "model/Document.h"
#include "model/DocumentHandler.h"

TEST(DocumentName, testUTF8) {
    DocumentHandler dh;
    Document doc(&dh);
    auto p = doc.createSaveFilename(Document::PDF, "%%ç测试ôê€ß", "%{name}测试");
    std::cout << p << std::endl;
    p = doc.createSaveFilename(Document::XOPP, "%%ç测试ôê€ß", "%{name}测试");
    std::cout << p << std::endl;
    doc.setFilepath("ùèçüûin/ë€ds测试q.xopp");
    p = doc.createSaveFilename(Document::PDF, "%%ç测试ôê€ß", "%{name}测试");
    std::cout << p << std::endl;
    p = doc.createSaveFilename(Document::XOPP, "%%ç测试ôê€ß", "%{name}测试");
    std::cout << p << std::endl;
}
