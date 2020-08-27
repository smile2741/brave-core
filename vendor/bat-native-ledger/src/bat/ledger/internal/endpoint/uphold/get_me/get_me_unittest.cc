/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <memory>
#include <string>
#include <vector>

#include "base/test/task_environment.h"
#include "bat/ledger/internal/ledger_client_mock.h"
#include "bat/ledger/internal/ledger_impl_mock.h"
#include "bat/ledger/internal/endpoint/uphold/get_me/get_me.h"
#include "bat/ledger/ledger.h"
#include "net/http/http_status_code.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=GetMeTest.*

using ::testing::_;
using ::testing::Invoke;

namespace ledger {
namespace endpoint {
namespace uphold {

class GetMeTest : public testing::Test {
 private:
  base::test::TaskEnvironment scoped_task_environment_;

 protected:
  std::unique_ptr<ledger::MockLedgerClient> mock_ledger_client_;
  std::unique_ptr<bat_ledger::MockLedgerImpl> mock_ledger_impl_;
  std::unique_ptr<GetMe> me_;

  GetMeTest() {
    mock_ledger_client_ = std::make_unique<ledger::MockLedgerClient>();
    mock_ledger_impl_ =
        std::make_unique<bat_ledger::MockLedgerImpl>(mock_ledger_client_.get());
    me_ = std::make_unique<GetMe>(mock_ledger_impl_.get());
  }
};

TEST_F(GetMeTest, ServerOK) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _, _, _, _, _))
      .WillByDefault(
          Invoke([](
              const std::string& url,
              const std::vector<std::string>& headers,
              const std::string& content,
              const std::string& contentType,
              const ledger::UrlMethod method,
              ledger::LoadURLCallback callback) {
            ledger::UrlResponse response;
            response.status_code = 200;
            response.url = url;
            response.body = R"({
             "address": {
               "city": "Anytown",
               "line1": "123 Main Street",
               "zipCode": "12345"
             },
             "birthdate": "1971-06-22",
             "country": "US",
             "email": "john@example.com",
             "firstName": "John",
             "fullName": "John Smith",
             "id": "b34060c9-5ca3-4bdb-bc32-1f826ecea36e",
             "identityCountry": "US",
             "lastName": "Smith",
             "name": "John Smith",
             "settings": {
               "currency": "USD",
               "hasMarketingConsent": false,
               "hasNewsSubscription": false,
               "intl": {
                 "dateTimeFormat": {
                   "locale": "en-US"
                 },
                 "language": {
                   "locale": "en-US"
                 },
                 "numberFormat": {
                   "locale": "en-US"
                 }
               },
               "otp": {
                 "login": {
                   "enabled": true
                 },
                 "transactions": {
                   "transfer": {
                     "enabled": false
                   },
                   "send": {
                     "enabled": true
                   },
                   "withdraw": {
                     "crypto": {
                       "enabled": true
                     }
                   }
                 }
               },
               "theme": "vintage"
             },
             "memberAt": "2019-07-27T11:32:33.310Z",
             "state": "US-MA",
             "status": "ok",
             "type": "individual",
             "username": null,
             "verifications": {
               "termsEquities": {
                 "status": "required"
               }
             },
             "balances": {
               "available": "3.15",
               "currencies": {
                 "BAT": {
                   "amount": "3.15",
                   "balance": "12.35",
                   "currency": "USD",
                   "rate": "0.25521"
                 }
               },
               "pending": "0.00",
               "total": "3.15"
             },
             "currencies": [
               "BAT"
             ],
             "phones": [
               {
                 "e164Masked": "+XXXXXXXXX83",
                 "id": "8037c7ed-fe5a-4ad2-abfd-7c941f066cab",
                 "internationalMasked": "+X XXX-XXX-XX83",
                 "nationalMasked": "(XXX) XXX-XX83",
                 "primary": false,
                 "verified": false
               }
             ],
             "tier": "other"
            })";
            callback(response);
          }));

  braveledger_uphold::User expected_user;


  me_->Request(
      "4c2b665ca060d912fec5c735c734859a06118cc8",
      [&](const ledger::Result result, const braveledger_uphold::User& user) {
        EXPECT_EQ(result, ledger::Result::LEDGER_OK);
        EXPECT_EQ(user.name, "John");
        EXPECT_EQ(user.member_at, "2019-07-27T11:32:33.310Z");
        EXPECT_EQ(user.verified, true);
        EXPECT_EQ(user.bat_not_allowed, false);
        EXPECT_EQ(user.status, braveledger_uphold::UserStatus::OK);
      });
}

TEST_F(GetMeTest, ServerError401) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _, _, _, _, _))
      .WillByDefault(
          Invoke([](
              const std::string& url,
              const std::vector<std::string>& headers,
              const std::string& content,
              const std::string& contentType,
              const ledger::UrlMethod method,
              ledger::LoadURLCallback callback) {
            ledger::UrlResponse response;
            response.status_code = 401;
            response.url = url;
            response.body = "";
            callback(response);
          }));

  braveledger_uphold::User expected_user;
  me_->Request(
      "4c2b665ca060d912fec5c735c734859a06118cc8",
      [&](const ledger::Result result, const braveledger_uphold::User& user) {
        EXPECT_EQ(result, ledger::Result::EXPIRED_TOKEN);
      });
}

TEST_F(GetMeTest, ServerErrorRandom) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _, _, _, _, _))
      .WillByDefault(
          Invoke([](
              const std::string& url,
              const std::vector<std::string>& headers,
              const std::string& content,
              const std::string& contentType,
              const ledger::UrlMethod method,
              ledger::LoadURLCallback callback) {
            ledger::UrlResponse response;
            response.status_code = 453;
            response.url = url;
            response.body = "";
            callback(response);
          }));

  braveledger_uphold::User expected_user;
  me_->Request(
      "4c2b665ca060d912fec5c735c734859a06118cc8",
      [&](const ledger::Result result, const braveledger_uphold::User& user) {
        EXPECT_EQ(result, ledger::Result::LEDGER_ERROR);
      });
}

}  // namespace uphold
}  // namespace endpoint
}  // namespace ledger