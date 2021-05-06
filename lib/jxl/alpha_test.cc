// Copyright (c) the JPEG XL Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lib/jxl/alpha.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace jxl {
namespace {

using ::testing::_;
using ::testing::ElementsAre;
using ::testing::FloatNear;

TEST(AlphaTest, BlendingWithNonPremultiplied) {
  const float bg_rgb[3] = {100, 110, 120};
  const float bg_a = 180.f / 255;
  const float fg_rgb[3] = {25, 21, 23};
  const float fg_a = 15420.f / 65535;
  float out_rgb[3];
  float out_a;
  PerformAlphaBlending(
      /*bg=*/{&bg_rgb[0], &bg_rgb[1], &bg_rgb[2], &bg_a},
      /*fg=*/{&fg_rgb[0], &fg_rgb[1], &fg_rgb[2], &fg_a},
      /*out=*/{&out_rgb[0], &out_rgb[1], &out_rgb[2], &out_a}, 1,
      /*alpha_is_premultiplied=*/false);
  EXPECT_THAT(out_rgb,
              ElementsAre(FloatNear(77.2f, .05f), FloatNear(83.0f, .05f),
                          FloatNear(90.6f, .05f)));
  EXPECT_NEAR(out_a, 3174.f / 4095, 1e-5);
}

TEST(AlphaTest, BlendingWithPremultiplied) {
  const float bg_rgb[3] = {100, 110, 120};
  const float bg_a = 180.f / 255;
  const float fg_rgb[3] = {25, 21, 23};
  const float fg_a = 15420.f / 65535;
  float out_rgb[3];
  float out_a;
  PerformAlphaBlending(
      /*bg=*/{&bg_rgb[0], &bg_rgb[1], &bg_rgb[2], &bg_a},
      /*fg=*/{&fg_rgb[0], &fg_rgb[1], &fg_rgb[2], &fg_a},
      /*out=*/{&out_rgb[0], &out_rgb[1], &out_rgb[2], &out_a}, 1,
      /*alpha_is_premultiplied=*/true);
  EXPECT_THAT(out_rgb,
              ElementsAre(FloatNear(101.5f, .05f), FloatNear(105.1f, .05f),
                          FloatNear(114.8f, .05f)));
  EXPECT_NEAR(out_a, 3174.f / 4095, 1e-5);
}

TEST(AlphaTest, AlphaWeightedAdd) {
  const float bg_rgb[3] = {100, 110, 120};
  const float bg_a = 180.f / 255;
  const float fg_rgb[3] = {25, 21, 23};
  const float fg_a = 1.f / 4;
  float out_rgb[3];
  float out_a;
  PerformAlphaWeightedAdd(
      /*bg=*/{&bg_rgb[0], &bg_rgb[1], &bg_rgb[2], &bg_a},
      /*fg=*/{&fg_rgb[0], &fg_rgb[1], &fg_rgb[2], &fg_a},
      /*out=*/{&out_rgb[0], &out_rgb[1], &out_rgb[2], &out_a}, 1);
  EXPECT_THAT(out_rgb, ElementsAre(FloatNear(100.f + 25.f / 4, .05f),
                                   FloatNear(110.f + 21.f / 4, .05f),
                                   FloatNear(120.f + 23.f / 4, .05f)));
  EXPECT_EQ(out_a, bg_a);
}

TEST(AlphaTest, Mul) {
  const float bg = 100;
  const float fg = 25;
  float out;
  PerformMulBlending(&bg, &fg, &out, 1);
  EXPECT_THAT(out, FloatNear(fg * bg, .05f));
}

TEST(AlphaTest, PremultiplyAndUnpremultiply) {
  const float alpha[] = {0.f, 63.f / 255, 127.f / 255, 1.f};
  float r[] = {120, 130, 140, 150};
  float g[] = {124, 134, 144, 154};
  float b[] = {127, 137, 147, 157};

  PremultiplyAlpha(r, g, b, alpha, 4);
  EXPECT_THAT(
      r, ElementsAre(FloatNear(0.f, 1e-5f), FloatNear(130 * 63.f / 255, 1e-5f),
                     FloatNear(140 * 127.f / 255, 1e-5f), 150));
  EXPECT_THAT(
      g, ElementsAre(FloatNear(0.f, 1e-5f), FloatNear(134 * 63.f / 255, 1e-5f),
                     FloatNear(144 * 127.f / 255, 1e-5f), 154));
  EXPECT_THAT(
      b, ElementsAre(FloatNear(0.f, 1e-5f), FloatNear(137 * 63.f / 255, 1e-5f),
                     FloatNear(147 * 127.f / 255, 1e-5f), 157));

  UnpremultiplyAlpha(r, g, b, alpha, 4);
  EXPECT_THAT(r, ElementsAre(FloatNear(120, 1e-4f), FloatNear(130, 1e-4f),
                             FloatNear(140, 1e-4f), FloatNear(150, 1e-4f)));
  EXPECT_THAT(g, ElementsAre(FloatNear(124, 1e-4f), FloatNear(134, 1e-4f),
                             FloatNear(144, 1e-4f), FloatNear(154, 1e-4f)));
  EXPECT_THAT(b, ElementsAre(FloatNear(127, 1e-4f), FloatNear(137, 1e-4f),
                             FloatNear(147, 1e-4f), FloatNear(157, 1e-4f)));
}

TEST(AlphaTest, UnpremultiplyAndPremultiply) {
  const float alpha[] = {0.f, 63.f / 255, 127.f / 255, 1.f};
  float r[] = {50, 60, 70, 80};
  float g[] = {54, 64, 74, 84};
  float b[] = {57, 67, 77, 87};

  UnpremultiplyAlpha(r, g, b, alpha, 4);
  EXPECT_THAT(r, ElementsAre(_, FloatNear(60 * 255.f / 63, 1e-4f),
                             FloatNear(70 * 255.f / 127, 1e-4f), 80));
  EXPECT_THAT(g, ElementsAre(_, FloatNear(64 * 255.f / 63, 1e-4f),
                             FloatNear(74 * 255.f / 127, 1e-4f), 84));
  EXPECT_THAT(b, ElementsAre(_, FloatNear(67 * 255.f / 63, 1e-4f),
                             FloatNear(77 * 255.f / 127, 1e-4f), 87));

  PremultiplyAlpha(r, g, b, alpha, 4);
  EXPECT_THAT(r, ElementsAre(FloatNear(50, 1e-4f), FloatNear(60, 1e-4f),
                             FloatNear(70, 1e-4f), FloatNear(80, 1e-4f)));
  EXPECT_THAT(g, ElementsAre(FloatNear(54, 1e-4f), FloatNear(64, 1e-4f),
                             FloatNear(74, 1e-4f), FloatNear(84, 1e-4f)));
  EXPECT_THAT(b, ElementsAre(FloatNear(57, 1e-4f), FloatNear(67, 1e-4f),
                             FloatNear(77, 1e-4f), FloatNear(87, 1e-4f)));
}

}  // namespace
}  // namespace jxl
