/* Copyright 2021 the SumatraPDF project authors (see AUTHORS file).
   License: Simplified BSD (see COPYING.BSD) */

// this is for adding temporary code for testing

#include "utils/BaseUtil.h"
#include "utils/Archive.h"
#include "utils/FileUtil.h"
#include "utils/WinUtil.h"
#include "utils/GdiPlusUtil.h"

#include "DisplayMode.h"
#include "Controller.h"
#include "PalmDbReader.h"
#include "EbookBase.h"
#include "EbookDoc.h"
#include "MobiDoc.h"
#include "Scratch.h"

#include "utils/Log.h"

// META-INF/container.xml
static const char* metaInfContainerXML = R"(<?xml version="1.0"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
   <rootfiles>
      <rootfile full-path="content.opf" media-type="application/oebps-package+xml"/>
      
   </rootfiles>
</container>)";

// mimetype

static const char* mimeType = R"(application/epub+zip)";

// content.opf
static const char* contentOpf = R"(<?xml version='1.0' encoding='utf-8'?>
<package xmlns="http://www.idpf.org/2007/opf" unique-identifier="uuid_id" version="2.0">
  <metadata xmlns:calibre="http://calibre.kovidgoyal.net/2009/metadata" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:opf="http://www.idpf.org/2007/opf" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
    <dc:title>input</dc:title>
  </metadata>
  <manifest>
    <item href="input.htm" id="html" media-type="application/xhtml+xml"/>
    <item href="toc.ncx" id="ncx" media-type="application/x-dtbncx+xml"/>
  </manifest>
  <spine toc="ncx">
    <itemref idref="html"/>
  </spine>
</package>
)";

// toc.ncx
static const char* tocNcx = R"--(<?xml version='1.0' encoding='utf-8'?>
<ncx xmlns="http://www.daisy.org/z3986/2005/ncx/" version="2005-1" xml:lang="en-US">
  <head>
    <meta content="d3bfbfa8-a0b5-4e1c-9297-297fb130bcbc" name="dtb:uid"/>
    <meta content="2" name="dtb:depth"/>
    <meta content="0" name="dtb:totalPageCount"/>
    <meta content="0" name="dtb:maxPageNumber"/>
  </head>
  <docTitle>
    <text>input</text>
  </docTitle>
  <navMap>
    <navPoint id="urvn6G6FwfJA4zq7INOgGaD" playOrder="1">
      <navLabel>
        <text>Start</text>
      </navLabel>
      <content src="input.htm"/>
    </navPoint>
  </navMap>
</ncx>
)--";

Vec<FileData*> MobiToEpub2(const WCHAR* path) {
    Vec<FileData*> res;
    MobiDoc* doc = MobiDoc::CreateFromFile(path);
    if (!doc) {
        return res;
    }
    auto d = doc->GetHtmlData();
    {
        auto e = new FileData();
        e->name = str::Dup("input.htm");
        e->data = d.Clone();
        res.Append(e);
        logf("name: '%s', size: %d, %d images\n", e->name, (int)e->data.size(), doc->imagesCount);
    }

    for (size_t i = 1; i <= doc->imagesCount; i++) {
        auto imageData = doc->GetImage(i);
        if (!imageData) {
            logf("image %d is missing\n", (int)i);
            continue;
        }
        const WCHAR* ext = GfxFileExtFromData(*imageData);
        char* extA = ToUtf8Temp(ext).Get();
        logf("image %d, size: %d, ext: %s\n", (int)i, (int)imageData->size(), extA);
        auto e = new FileData();
        e->name = str::Format("image-%d%s", (int)i, extA);
        e->data = imageData->Clone();
        e->imageNo = (int)i;
        res.Append(e);
    }
    return res;
}

Vec<FileData*> MobiToEpub(const WCHAR* path) {
    auto files = MobiToEpub2(path);
    const WCHAR* dstDir = LR"(C:\Users\kjk\Downloads\mobiToEpub)";
    for (auto& f : files) {
        WCHAR* name = ToWstrTemp(f->name);
        AutoFreeWstr dstPath = path::Join(dstDir, name);
        bool ok = file::WriteFile(dstPath.Get(), f->data);
        if (!ok) {
            logf("Failed to write '%s'\n", ToUtf8Temp(dstPath).Get());
        } else {
            logf("Wrote '%s'\n", ToUtf8Temp(dstPath).Get());
        }
    }
    return files;
}
