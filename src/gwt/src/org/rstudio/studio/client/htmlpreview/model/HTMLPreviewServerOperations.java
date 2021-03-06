/*
 * HTMLPreviewServerOperations.java
 *
 * Copyright (C) 2009-12 by RStudio, Inc.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */
package org.rstudio.studio.client.htmlpreview.model;

import org.rstudio.core.client.files.FileSystemItem;
import org.rstudio.studio.client.server.Void;
import org.rstudio.studio.client.server.ServerRequestCallback;
import org.rstudio.studio.client.workbench.model.HTMLCapabilities;

public interface HTMLPreviewServerOperations
{
   void previewHTML(HTMLPreviewParams params, 
                    ServerRequestCallback<Boolean> requestCallback);

   void terminatePreviewHTML(ServerRequestCallback<Void> requestCallback);

   void getHTMLCapabilities(ServerRequestCallback<HTMLCapabilities> callback);

   // copy file
   void copyFile(FileSystemItem sourceFile, 
                 FileSystemItem targetFile,
                 boolean overwrite, 
                 ServerRequestCallback<Void> requestCallback);

   String getApplicationURL(String pathName);
   String getFileUrl(FileSystemItem file);
   
   // rpubs
   void rpubsUpload(String title, 
                    String htmlFile,
                    ServerRequestCallback<Boolean> requestCallback);
   
   void rpubsTerminateUpload(ServerRequestCallback<Void> requestCallback);
}
