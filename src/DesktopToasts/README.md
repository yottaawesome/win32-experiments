---
page_type: sample
languages:
- cpp
- csharp
products:
- windows-api-win32
name: Basic desktop-toast sample
urlFragment: desktop-toasts
description: Demonstrates how a desktop app can display a toast notification.
---

# Desktop toasts sample

This includes samples for desktop toasts using com activation. The sample demonstrates how a desktop app can display a toast notification and respond to events generated by the user's interaction (or lack of interaction) with the toast. 

This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs.

## How to use the sample

1. In the sample window, select **View Text Toast**. A sample toast notification appears on the screen. 
1. As the user, there are three actions you can take: 
   * Touch or select the toast. In a real-world app, this would commonly launch the associated app. 
   * Horizontally swipe the toast to dismiss it. 
   * Do nothing. The toast will time out. 
   
The action you take displays in the sample window to demonstrate the receipt of the event. 

In a real-world situation, your app would respond appropriately to each of these events. Be aware that for a desktop app to be able to display toast notifications, that app must have a shortcut on the **Start** menu. That shortcut must have an **AppUserModelID**. 

## Related Topics

For more information see https://blogs.msdn.microsoft.com/tiles_and_toasts/2015/10/16/quickstart-handling-toast-activations-from-win32-apps-in-windows-10/

To load the `SetupProject.sln`, install the WIX tool set from http://wixtoolset.org/releases/.
