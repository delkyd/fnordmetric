/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <fnordmetric/environment.h>
#include <fnordmetric/io/fileutil.h>
#include <fnordmetric/metricdb/backends/disk/metric.h>
#include <fnordmetric/util/unittest.h>
#include <fnordmetric/util/wallclock.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace fnordmetric::metricdb::disk_backend;
using namespace fnordmetric::metricdb;
using namespace fnord::io;

UNIT_TEST(DiskBackendTest);

const char kTestRepoPath[] = "/tmp/__fnordmetric_test_metricrepo2";

using LabelListType = std::vector<std::pair<std::string, std::string>>;

TEST_CASE(DiskBackendTest, TestCreateNewMetric, [] () {
  io::FileUtil::mkdir_p(kTestRepoPath);
  FileRepository file_repo(kTestRepoPath);
  file_repo.deleteAllFiles();

  Metric metric("myfirstmetric", &file_repo);
  metric.setLiveTableMaxSize(4 * 2 << 19); /* 4MB */

  int n = 0;
  metric.scanSamples(
      util::DateTime::epoch(),
      util::DateTime::now(),
      [&n] (Sample* sample) -> bool {
        n++;
        return true;
      });

  EXPECT_EQ(n, 0);

  std::vector<std::string> labels;
  for (int i = 0; i < 1000; ++i) {
    labels.emplace_back(fnord::util::Random::alphanumericString(16));
  }

  auto seq1 = [] (double x) {
    return fmod((x + 1) * 23.5f, 4200.0f);
  };

  int num_saples = 100000;
  for (int i = 0; i < num_saples; ++i) {
    LabelListType smpl_labels;
    smpl_labels.emplace_back(labels[i % labels.size()], "myvalue");
    metric.insertSample(seq1(i), smpl_labels);
  }

  size_t total_bytes = metric.totalBytes();
  metric.compact();
  EXPECT_EQ(metric.totalBytes(), total_bytes);

  n = 0;
  metric.scanSamples(
      util::DateTime::epoch(),
      util::DateTime::now(),
      [&n, &seq1] (Sample* sample) -> bool {
        EXPECT_EQ(sample->value(), seq1(n));
        n++;
        return true;
      });

  EXPECT_EQ(n, num_saples);
});




