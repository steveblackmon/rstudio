/*
 * SessionProjects.cpp
 *
 * Copyright (C) 2009-11 by RStudio, Inc.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */

// TODO: detecting copy/move/network:
/*
    - read INDEX
    - read id file

    - if INDEX contains id mapped to correct path then use

    - if INDEX doesn't not contain id or path then create new

    - if INDEX has id but path doesn't match then may have been
      a move or a copy so create a copy of the scratch dir and
      associate it with a new id

    - if has an id file but nothing in the index at all then
      create a brand new entry/dir using that id
*/



#include <session/projects/SessionProjects.hpp>

#include <core/FilePath.hpp>
#include <core/Settings.hpp>
#include <core/Exec.hpp>
#include <core/FileSerializer.hpp>
#include <core/system/System.hpp>
#include <core/FileSerializer.hpp>
#include <core/r_util/RProjectFile.hpp>

#include <session/SessionModuleContext.hpp>
#include <session/SessionUserSettings.hpp>
#include <session/SessionPersistentState.hpp>

using namespace core;

namespace session {
namespace projects {

namespace {

ProjectContext s_projectContext;


void onSuspend(Settings*)
{
   // on suspend write out current project path as the one to use
   // on resume. we read this back in initalize (rather than in
   // the onResume handler) becuase we need it very early in the
   // processes lifetime and onResume happens too late
   persistentState().setNextSessionProjectPath(s_projectContext.file());
}

void onResume(const Settings&) {}

Error createProject(const json::JsonRpcRequest& request,
                    json::JsonRpcResponse* pResponse)
{
   // determine project file path
   std::string projectFile;
   Error error = json::readParam(request.params, 0, &projectFile);
   if (error)
      return error;
   FilePath projectFilePath = module_context::resolveAliasedPath(projectFile);

   // create the project file
   return r_util::writeProjectFile(projectFilePath);
}

}  // anonymous namespace


void startup()
{
   // register suspend handler
   using namespace module_context;
   addSuspendHandler(SuspendHandler(onSuspend, onResume));

   // determine project file path
   FilePath projectFilePath;

   // see if there is a project path hard-wired for the next session
   // (this would be used for a switch to project or for the resuming of
   // a suspended session)
   FilePath nextSessionProjectPath = persistentState().nextSessionProjectPath();
   if (!nextSessionProjectPath.empty())
   {
      // reset next session project path so its a one shot deal
      persistentState().setNextSessionProjectPath(FilePath());

      // clear any initial context settings which may be leftover
      // by a re-instatiation of rsession by desktop
      session::options().clearInitialContextSettings();

      // set project path
      projectFilePath = nextSessionProjectPath;
   }

   // check for envrionment variable (file association)
   else if (!session::options().initialProjectPath().empty())
   {
      projectFilePath = session::options().initialProjectPath();
   }

   // check for other working dir override (implies a launch of a file
   // but not of a project). this code path is here to prevent
   // the next code path from executing
   else if (!session::options().initialWorkingDirOverride().empty())
   {
      projectFilePath = FilePath();
   }

   // check for restore based last project (but surpress this if we
   // had an abend on our last session -- this might have been a
   // result of something in the project)
   else if (userSettings().alwaysRestoreLastProject() &&
            !userSettings().lastProjectPath().empty() &&
            !session::persistentState().hadAbend())
   {
      // get last project path
      projectFilePath = userSettings().lastProjectPath();

      // reset it to empty so that we only attempt to load the "lastProject"
      // a single time (this will be reset to the path below after we
      // clear the s_projectContext.initialize)
      userSettings().setLastProjectPath(FilePath());
   }

   // else no active project for this session
   else
   {
      projectFilePath = FilePath();
   }

   // if we have a project file path then try to initialize the
   // project context (show a warning to the user if we can't)
   if (!projectFilePath.empty())
   {
      std::string userErrMsg;
      Error error = s_projectContext.initialize(projectFilePath, &userErrMsg);
      if (error)
      {
         // log the error
         error.addProperty("project-file", projectFilePath.absolutePath());
         error.addProperty("user-msg", userErrMsg);
         LOG_ERROR(error);

         // show the error
         std::string msg =
           "Project '" + projectFilePath.absolutePath() + "' "
           "could not be opened: " + userErrMsg;
          module_context::showErrorMessage("Error Opening Project", msg);
      }
   }

   // save the active project path for the next session (may be empty)
   userSettings().setLastProjectPath(s_projectContext.file());
}

Error initialize()
{
   using boost::bind;
   using namespace module_context;
   ExecBlock initBlock ;
   initBlock.addFunctions()
      (bind(registerRpcMethod, "create_project", createProject))
   ;
   return initBlock.execute();
}

const ProjectContext& projectContext()
{
   return s_projectContext;
}

} // namespace projects
} // namesapce session
