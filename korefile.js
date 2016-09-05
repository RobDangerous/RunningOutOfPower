var project = new Project('BlocksFromHeaven', __dirname);

project.addFile('Sources/**');
project.setDebugDir('Deployment');

project.addSubProject(Project.createProject('Kore', __dirname));

resolve(project);
