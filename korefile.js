var project = new Project('RunningOutOfPower', __dirname);

project.addFile('Sources/**');
project.setDebugDir('Deployment');

Project.createProject('Kore', __dirname).then((subproject) => {
	project.addSubProject(subproject);
	resolve(project);
});
