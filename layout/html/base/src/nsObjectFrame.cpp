/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */
#include "nsHTMLParts.h"
#include "nsLeafFrame.h"
#include "nsCSSLayout.h"
#include "nsIPresContext.h"
#include "nsIPresShell.h"
#include "nsWidgetsCID.h"
#include "nsViewsCID.h"
#include "nsIView.h"
#include "nsIViewManager.h"
#include "nsplugin.h"
#include "nsIPluginHost.h"
#include "nsString.h"
#include "prmem.h"

#define nsObjectFrameSuper nsLeafFrame

class nsObjectFrame : public nsObjectFrameSuper {
public:
  nsObjectFrame(nsIContent* aContent, nsIFrame* aParentFrame);

  NS_IMETHOD Reflow(nsIPresContext&      aPresContext,
                    nsReflowMetrics&     aDesiredSize,
                    const nsReflowState& aReflowState,
                    nsReflowStatus&      aStatus);

  NS_IMETHOD DidReflow(nsIPresContext& aPresContext,
                       nsDidReflowStatus aStatus);

protected:
  virtual ~nsObjectFrame();

  virtual void GetDesiredSize(nsIPresContext* aPresContext,
                              const nsReflowState& aReflowState,
                              nsReflowMetrics& aDesiredSize);

  nsresult CreateWidget(nsIPresContext* aPresContext,
                        nscoord aWidth, nscoord aHeight);
};

nsObjectFrame::nsObjectFrame(nsIContent* aContent, nsIFrame* aParentFrame)
  : nsObjectFrameSuper(aContent, aParentFrame)
{
}

nsObjectFrame::~nsObjectFrame()
{
}

static NS_DEFINE_IID(kViewCID, NS_VIEW_CID);
static NS_DEFINE_IID(kIViewIID, NS_IVIEW_IID);
static NS_DEFINE_IID(kWidgetCID, NS_CHILD_CID);

nsresult
nsObjectFrame::CreateWidget(nsIPresContext* aPresContext,
                            nscoord aWidth, nscoord aHeight)
{
  nsIView* view;

  // Create our view and widget

  nsresult result = 
    NSRepository::CreateInstance(kViewCID, nsnull, kIViewIID,
				 (void **)&view);
  if (NS_OK != result) {
    return result;
  }
  nsIPresShell   *presShell = aPresContext->GetShell();     // need to release
  nsIViewManager *viewMan   = presShell->GetViewManager();  // need to release

  nsRect boundBox(0, 0, aWidth, aHeight); 

  nsIFrame* parWithView;
  nsIView *parView;

  GetParentWithView(parWithView);
  parWithView->GetView(parView);

//  nsWidgetInitData* initData = GetWidgetInitData(*aPresContext); // needs to be deleted
  // initialize the view as hidden since we don't know the (x,y) until Paint
  result = view->Init(viewMan, boundBox, parView, &kWidgetCID, nsnull,
                      nsnull, 0, nsnull,
                      1.0f, nsViewVisibility_kHide);
//  if (nsnull != initData) {
//    delete(initData);
//  }
  if (NS_OK != result) {
    return NS_OK;
  }

#if 0
  // set the content's widget, so it can get content modified by the widget
  nsIWidget *widget;
  result = GetWidget(view, &widget);
  if (NS_OK == result) {
    nsInput* content = (nsInput *)mContent; // change this cast to QueryInterface 
    content->SetWidget(widget);
    NS_IF_RELEASE(widget);
  } else {
    NS_ASSERTION(0, "could not get widget");
  }
#endif

  viewMan->InsertChild(parView, view, 0);

  SetView(view);
  NS_RELEASE(view);
	  
  NS_IF_RELEASE(parView);
  NS_IF_RELEASE(viewMan);  
  NS_IF_RELEASE(presShell); 
  return result;
}

#define EMBED_DEF_DIM 50

void
nsObjectFrame::GetDesiredSize(nsIPresContext* aPresContext,
                              const nsReflowState& aReflowState,
                              nsReflowMetrics& aMetrics)
{
  // Determine our size stylistically
  nsSize styleSize;
  PRIntn ss = nsCSSLayout::GetStyleSize(aPresContext, aReflowState, styleSize);
  PRBool haveWidth = PR_FALSE;
  PRBool haveHeight = PR_FALSE;
  if (0 != (ss & NS_SIZE_HAS_WIDTH)) {
    aMetrics.width = styleSize.width;
    haveWidth = PR_TRUE;
  }
  if (0 != (ss & NS_SIZE_HAS_HEIGHT)) {
    aMetrics.height = styleSize.height;
    haveHeight = PR_TRUE;
  }

  // XXX Temporary auto-sizing logic
  if (!haveWidth) {
    if (haveHeight) {
      aMetrics.width = aMetrics.height;
    }
    else {
      float p2t = aPresContext->GetPixelsToTwips();
      aMetrics.width = nscoord(p2t * EMBED_DEF_DIM);
    }
  }
  if (!haveHeight) {
    if (haveWidth) {
      aMetrics.height = aMetrics.width;
    }
    else {
      float p2t = aPresContext->GetPixelsToTwips();
      aMetrics.height = nscoord(p2t * EMBED_DEF_DIM);
    }
  }
  aMetrics.ascent = aMetrics.height;
  aMetrics.descent = 0;
}

NS_IMETHODIMP
nsObjectFrame::Reflow(nsIPresContext&      aPresContext,
                      nsReflowMetrics&     aMetrics,
                      const nsReflowState& aReflowState,
                      nsReflowStatus&      aStatus)
{
  // Get our desired size
  GetDesiredSize(&aPresContext, aReflowState, aMetrics);
  if (nsnull != aMetrics.maxElementSize) {
    aMetrics.maxElementSize->width = aMetrics.width;
    aMetrics.maxElementSize->height = aMetrics.height;
  }

  // XXX deal with border and padding the usual way...wrap it up!

  // Create view if necessary
  nsIView* view;
  GetView(view);
  if (nsnull == view) {
    nsresult rv = CreateWidget(&aPresContext, aMetrics.width,
                               aMetrics.height);
    if (NS_OK != rv) {
      return rv;
    }

    static NS_DEFINE_IID(kIPluginHostIID, NS_IPLUGINHOST_IID);

    nsISupports    *container, *pluginsup;
    nsIPluginHost  *pm;

    rv = aPresContext.GetContainer(&container);

    if (NS_OK == rv) {
      rv = container->QueryInterface(kIPluginHostIID, (void **)&pm);

      if (NS_OK == rv) {
        nsString  type;
        char      *buf;
        PRInt32   buflen;

        mContent->GetAttribute(nsString("type"), type);

        buflen = type.Length();

        if (buflen > 0) {
          buf = (char *)PR_Malloc(buflen + 1);

          if (nsnull != buf) {
            type.ToCString(buf, buflen + 1);

            rv = pm->InstantiatePlugin(buf, &pluginsup);

            if (NS_OK == rv) {
            }
          }
        }

        NS_RELEASE(pm);
      }

      NS_RELEASE(container);
    }
  }
  else {
    NS_RELEASE(view);
  }

  aStatus = NS_FRAME_COMPLETE;
  return NS_OK;
}

NS_IMETHODIMP
nsObjectFrame::DidReflow(nsIPresContext& aPresContext,
                         nsDidReflowStatus aStatus)
{
  nsresult rv = nsObjectFrameSuper::DidReflow(aPresContext, aStatus);

  // The view is created hidden; once we have reflowed it and it has been
  // positioned then we show it.
  if (NS_FRAME_REFLOW_FINISHED == aStatus) {
    nsIView* view = nsnull;
    GetView(view);
    if (nsnull != view) {
      view->SetVisibility(nsViewVisibility_kShow);
      NS_RELEASE(view);
    }
  }
  return rv;
}

nsresult
NS_NewObjectFrame(nsIFrame*& aFrameResult, nsIContent* aContent,
                  nsIFrame* aParentFrame)
{
  aFrameResult = new nsObjectFrame(aContent, aParentFrame);
  if (nsnull == aFrameResult) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}
