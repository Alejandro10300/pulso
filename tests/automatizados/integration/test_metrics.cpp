#include <gtest/gtest.h>
#include <httplib.h>

TEST(IntegrationMetricsTest, GetMetricsReturns200) {
    httplib::Client cli("localhost", 8080);

    auto res = cli.Get("/metrics");

    ASSERT_TRUE(res != nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->get_header_value("Content-Type"), "application/json");
    EXPECT_NE(res->body.find("timestamp"), std::string::npos);
    EXPECT_NE(res->body.find("metricas"), std::string::npos);
    EXPECT_NE(res->body.find("nombre"), std::string::npos);
}
