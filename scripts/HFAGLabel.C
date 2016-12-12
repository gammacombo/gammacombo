#include "TString.h"
#include "TPad.h"
#include "TPaveText.h"

void HFAGLabel(const TString& label="please set label",
		  // Double_t xpos=0.6, Double_t ypos=0.2,
		  Double_t xpos=0, Double_t ypos=0,
		  Double_t scale=1);

void HFAGLabel(const TString& label,
		  Double_t xpos, Double_t ypos,
		  Double_t scale)
{
  TVirtualPad* thePad;

  if ((thePad = TVirtualPad::Pad()) == 0) return;

  UInt_t pad_width(thePad->XtoPixel(thePad->GetX2()));
  UInt_t pad_height(thePad->YtoPixel(thePad->GetY1()));

  Double_t ysiz_pixel(25);
  Double_t ysiz(Double_t(ysiz_pixel)/Double_t(pad_height));
  Double_t xsiz(4.8*ysiz*Double_t(pad_height)/Double_t(pad_width));

  Double_t x1, x2, y1, y2;
  xsiz = scale*xsiz;
  ysiz = scale*ysiz;

  if (xpos >= 0) {
    x1 = xpos;
    x2 = xpos + xsiz;
  } else {
    x1 = 1 + xpos - xsiz;
    x2 = 1 + xpos;
  }

  if (ypos >= 0) {
    y1 = ypos+0.9*ysiz;
    y2 = ypos+0.9*ysiz + ysiz;
  } else {
    y1 = 1 + ypos - ysiz;
    y2 = 1 + ypos;
  }

  TPaveText *tbox1 = new TPaveText(x1, y1, x2, y2, "BRNDC");
  // tbox1->SetLineColor(1);
  // tbox1->SetLineStyle(1);
  // tbox1->SetLineWidth(2);
  tbox1->SetFillColor(kBlack);
  tbox1->SetFillStyle(1001);
  // tbox1->SetBorderSize(1);
  tbox1->SetShadowColor(kWhite);
  tbox1->SetTextColor(kWhite);
  tbox1->SetTextFont(76);
  tbox1->SetTextSize(24*scale);
  tbox1->SetTextAlign(22); //center-adjusted and vertically centered
  tbox1->AddText(TString("HFAG"));
  tbox1->Draw();
  //
  TPaveText *tbox2 = new TPaveText(x1, y1-0.9*ysiz, x2, y2-ysiz, "BRNDC");
  // tbox2->SetLineColor(1);
  // tbox2->SetLineStyle(1);
  // tbox2->SetLineWidth(2);
  tbox2->SetFillColor(kWhite);
  tbox2->SetFillStyle(1001);
  // tbox2->SetBorderSize(1);
  tbox2->SetShadowColor(kWhite);
  tbox2->SetTextColor(kBlack);
  tbox2->SetTextFont(76);
  tbox2->SetTextSize(18*scale);
  tbox2->SetTextAlign(22); //center-adjusted and vertically centered
  tbox2->AddText(label);
  tbox2->Draw();
  return;
}

