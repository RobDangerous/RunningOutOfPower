var solution = new Solution('BlocksFromHeaven');
var project = new Project('BlocksFromHeaven');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

project.addSubProject(Solution.createProject('Kore'));

solution.addProject(project)

return solution;
