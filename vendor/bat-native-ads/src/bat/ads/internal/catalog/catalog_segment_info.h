/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_CATALOG_CATALOG_SEGMENT_INFO_H_
#define BAT_ADS_INTERNAL_CATALOG_CATALOG_SEGMENT_INFO_H_

#include <string>
#include <vector>

namespace ads {

struct CatalogSegmentInfo {
  CatalogSegmentInfo();
  CatalogSegmentInfo(
      const CatalogSegmentInfo& info);
  ~CatalogSegmentInfo();

  bool operator==(
      const CatalogSegmentInfo& rhs) const;
  bool operator!=(
      const CatalogSegmentInfo& rhs) const;

  std::string code;
  std::string name;
};

using CatalogSegmentList = std::vector<CatalogSegmentInfo>;

}  // namespace ads

#endif  // BAT_ADS_INTERNAL_CATALOG_CATALOG_SEGMENT_INFO_H_
