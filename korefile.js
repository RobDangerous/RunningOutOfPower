var project = new Project('BlocksFromHeaven');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

project.addSubProject(Project.createProject('Kore'));

return project;
