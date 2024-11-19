![sonar](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/6bc73c73-2d47-43ca-ab4f-6807167363e3)

<pre>
  TABLE OF CONTENTS
Launch Instances
Install Jenkins
Creating a Pipeline in Jenkins Server
Adding a Webhooks
Starting a SonarQube Server
Installing Docker
Password-based Server Connectivity
Building the Image and running the container
</pre>


<h1>Launch Instances</h1>
<p>
  I have created 3 t2.medium instances
  <li>Instance for Jenkins</li>
  <li>Instance for Sonarqube</li>
  <li>Instance for Docker</li>
</p>

![kl](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/c9e8adbe-d2b3-4b53-9753-cbecaa327c8d)

<h1>Install Jenkins</h1>
<p>Update the system</p>
<pre>
  sudo apt update
  clear
</pre>

Now we have to install java first before installing Jenkins
<pre>
  sudo apt install openjdk-11-jre -y
</pre>

Now we have to paste the Jenkins commands
<pre>
   curl -fsSL https://pkg.jenkins.io/debian-stable/jenkins.io-2023.key | sudo tee \
   /usr/share/keyrings/jenkins-keyring.asc > /dev/null
   echo deb [signed-by=/usr/share/keyrings/jenkins-keyring.asc] \
   https://pkg.jenkins.io/debian-stable binary/ | sudo tee \
  /etc/apt/sources.list.d/jenkins.list > /dev/null
  sudo apt-get update
  sudo apt-get install jenkins
</pre>

Now we have to allow port 8080 in the Jenkins server security group inbound settings

![dfa34375-6e63-4d8d-89cb-0f80ac0947b3](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/f2d57f02-4e9e-4a35-baf0-3eff0268e81b)

Now use the public api of instance with port 8080 to view the Jenkins server

![26f464b3-38c6-44a6-8c64-efe30006a062](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/1268d46a-289b-4fa4-8414-9b287b7ce8e3)

Copy the command which was highlighted in the above picture and use the command
<pre>
  sudo cat /var/lib/jenkins/secrets/initialAdminPassword
</pre>

It will show the output a password for the Jenkins server

![b5e9e13c-c112-4a7d-814b-c07d59cc7f54](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/8271624a-7025-4c15-b1cd-cad2b5cbae68)

Copy the password and paste it to the Jenkins server

![8a19b65f-eb48-4207-93e3-50beb321bde0](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/79701bae-1fd1-4c53-9e33-1f4c01eff9c1)

Click on -> Install suggested plugins
<br>
You can set admin credentials

![b3f4de45-9dd9-4bb9-b0c4-484d164cb15b](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/57a3607f-06da-4cd4-b839-c5f30cfd537f)


**Jenkins Installation done**

![041a6a53-4f76-430a-9319-73809d38c2bf](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/cafd5c48-69d0-447d-9914-3ebb24882190)

<h1>Creating a Pipeline in Jenkins Server</h1>
In this, I have selected the Freestyle project

![11c93238-30ac-4bc6-b012-d79f966ee0b4](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/060e8e19-93d7-4693-b47a-f0c1fb958cfd)

In the source code management, I have selected the Git option

![66935e93-ec40-4750-a131-e047a6d8518f](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/263cc9bf-acf7-4acc-9052-7793a71d52b5)

Also select branch of your git

![8775a904-c2b2-4e3f-968e-f417bd256bb9](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/83fdc997-92c0-4ddd-965f-204b6a3bcd8a)

<h1>Adding a Webhooks</h1>

Now open the settings of your Git Repository, select webhooks

![scre1](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/b1f31e4b-6b68-41eb-b925-7e7cf1cbde7f)

Click to Add Webhook, that time it will ask you to enter your password, <br>
once you enter your password, you're able to add Webhooks.

<br>
<li>Enter the Jenkins URL such as http://35.175.189.203:8080/</li>
<li>and extension github-webhook after the URL</li>



![screen1](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/fbcf9826-3798-4e95-9a07-663da68a10ed)

Click to Add Webhook

![screen2](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/121b110b-c838-47b4-a2db-51126596b1e0)


Now in Build Triggers, select the GitHub hook trigger for GITScm polling <br>
(because this function can trigger the pipeline automatically whenever we make changes to the repository.


<br>

![171388f4-083d-44d5-b14d-ce5fb0a97a2f](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/10261270-bb91-4223-ac7a-bf4db9d445df)

<br>
Click to save.
<br>
Without Webhook I have clicked to Build now and it is working perfectly fine

![404296de-eab1-4716-b32d-dca80826936b](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/536338e6-d101-41d0-906a-1dd72452191a)



Now time to verify Webhooks, I have clicked on the workspace in Jenkins, <br>
and here text.txt file is not present which I am going to create to test the Webhook

![1574a688-abf8-4d83-865f-3ff2f1a44ac7](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/8a751d93-e230-4ac9-af80-298bd3a9b061)


I have visited the Git Repository and created a new file by the name of test.txt


![Capture](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/21b8d1d0-ef66-45ac-bf1b-05073802e276)


I committed the file
<br>
Now I am back to the Jenkins server to check #2 build auto trigger and it is working fine.

![4ae8d251-1475-4e18-ae55-f56265f7a56c](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/871a81cf-706a-4d70-aef8-bbdd98add519)

![26f8a021-1f65-48fb-bca9-143934b82fa9](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/8546315e-ad9f-4c9c-9e6c-f859cca42540)

Now test.txt file is showing in the workspace

![48cb2544-1d36-43e4-a784-c07125771cf7](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/d1502abb-2664-4f20-9fb4-dc734c3f0681)


<h1>Starting a SonarQube Server</h1>
Start the Sonarqube Instance

<pre>
  sudo apt update
  clear
</pre>

We need to install Java on the server but 17 version earlier we were using 11 version

<pre>
  sudo apt install openjdk-17-jre -y
</pre>

Search for the SonarQube website and download the community for the free version<br>

SonarQube Download Copy the link and paste it to the terminal of the SonarQube Instance

<br>

![a097c83d-a980-4906-9279-763b2e385ea7](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/88c535bf-007b-4631-9753-1c30feead73d)

<pre>sudo adduser sonarqube
</pre>

![4a6cc6dc-fb48-4645-9548-bced7f50ccec](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/049ca112-76fc-4a3e-bdb3-24d789e6cbe8)

Use the command below with wget

<pre>
  wget https://binaries.sonarsource.com/Distribution/sonarqube/sonarqube-9.4.0.54424.zip
</pre>


![17688fc3-752f-4eb5-8afb-e59d487c385c](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/34a0faf3-461e-4c94-bde2-6a6f114297ca)


<pre>
  sudo apt install unzip
  unzip *
  ls
</pre>

Now the unzipped file is showing

![9d2ee9e6-e2d5-4dde-8f2c-7c98ff07db5b](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/878f451f-58ad-48e0-a673-60be2213296b)

<pre>
  cd sonarqube-9.4.0.54424 
ls
</pre>

![7652b9ab-5f85-47dc-a623-74beec0c2a52](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/f84ac169-c437-4530-813c-55e02c6f738f)


Go to the bin folder
<pre>
  cd bin
  cd linux-x86-64
  ls
</pre>
![c611fd78-564a-4f99-b1a5-3022d670b404](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/1724531e-3afa-4a83-8852-b5e6c08f64fe)


Now start the sonar server by using the command

<pre>./sonar.sh start</pre>

And also we need to port 9000 in the inbound setting of the security group

![f71b771d-2e6b-4222-9fba-16257693f97c](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/569052ab-1e16-4654-8e43-f77c09045cf8)


Now it's time to check the URL http://54.88.67.190:9000/

![aebca526-2027-4d5e-9db1-6669e38a6e0f](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/bcfe9563-63d1-4ee2-bc59-df1a75cd6617)

Update the password

![eb284491-e258-4175-b1fa-426dc7d1ecf7](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/d0b60e2e-7deb-4765-b9a3-05c633e4324f)

After updating the password we can access the Sonarqube

<h1>Follow Me on GitHub for more Projects</h1>

![c357ffa8-b7b4-456a-b0e8-38b0d8e5c32e](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/72c25d73-dc11-4063-8995-206b27115e52)

Now I have selected the project type Manually

![3e18d31f-db24-42e8-9acc-ead266df5dac](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/a2dda2b2-2a10-4086-baac-afe3617c8741)

After this, I have selected the CI platform, Jenkins

![437ac7bb-2cc7-45d2-8bcd-e5c174a7b181](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/3f1ad367-d055-435b-91d3-a638b494866c)

Selected the DevOps platform GitHub

![e77a9a76-7892-499e-b0ec-6ce9361916ed](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/9c21c00c-c355-48fa-99f3-210e329cd12b)


You can just simply select the options Configure Analysis



![db0b7319-bfd1-4a70-999b-12992a580ea6](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/68dd72ee-c344-4a06-a000-f619258eee11)
Then select Continue


![3fd08744-ed4b-4bdc-b36c-7fde63de368a](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/2072d566-77ad-41e5-84b2-a67ca0e6f42c)

Also, click on Continue here

![a9e8783c-2ce7-4f17-9fc3-f61d351a9048](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/9714ab53-733d-43cb-a079-90375d473428)

From here I have selected the Others

![af8aff44-1aed-4fd9-9558-b093ade1e739](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/d0635791-9d37-433c-aa3b-9521cb8606ea)

Copy the ProjectKey for now you can keep this Key in your notepad


<pre>sonar.projectKey=Sample-website</pre>

Finish the tutorial

![f035e680-dfe2-41a7-b1aa-15144a0bf77c](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/d7dc2a0f-c18c-4328-b933-1735a10d61c0)

Now click to settings, we need to create token

![b5ed0424-cf55-463b-a61b-a7426d134e3a](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/8a4cb930-cc08-478e-89a8-48e93bae566b)

Now I have created the Token and copy the token for now you can keep this token in your notepad

![f7fe49a0-bd90-42fa-8eec-b61ff3c3c9d0](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/b64693b7-13c4-4a31-ad55-3dae09c34b5d)

Now back to Jenkins, Manage Jenkins and Install the plugin

![c2721c5c-2872-4348-ba76-feccf58e5229](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/8225440d-b4cb-4d3b-9ee9-b504ed02de95)

Install one more plugin

![ce35b5e1-f610-413a-b77c-3d9e626ef9c7](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/b6473abc-5708-4ab7-8820-071ee69fa516)


Restart Jenkins after installation of plugins
<br>
Now Go to the Global Tool Configuration

<li>Scrolling Down</li>

<li>Click on SonarQube Scanner</li>

<li>Give any name and rest default</li>

![8e21a183-c392-4c7c-b8bc-d8febb5eabef](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/f21deae7-3fdb-40ae-9147-9528735da719)

After saving it, Click to Manage Jenkins again and click on Configure System


![a5254716-8d78-4868-868c-1361e24ba543](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/aaf97ff6-203d-48cf-82e5-ef35f1003f15)



Configure System

<li>Scrolling down</li>

<li>Click on the SonarQube server</li>

<li>Click to add</li>

<li>Give any name</li>

<li>Paste the Sonarqube URL</li>

Save


![4226949e-573b-43bc-b393-7481d3f02d7f](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/1b26c676-fdaa-4c47-9fdb-5851293e7494)

After Saving, Click Configure of the Pipeline in Jenkins

<li>Click to Build Environment</li>

<li>Select Execute SonarQube Scanner in Build Steps</li>

![2d2e35f5-1eba-40bb-9ecd-52c3201c1b77](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/8f4e10f8-5e76-4938-a712-0101b43faacd)

Ignore everything just paste the key here rest default and saved it

![1776e453-a5df-4016-ae70-d323fd0ef8f0](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/f9e23989-130f-4355-a950-cdf5805f0704)

Now back to Manage Jenkins, select Configure System again


![d661e861-8799-43b1-8bf7-fe03881a5e02](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/b8f0e557-c257-4dc5-ae52-f8edb624b84a)

Now we have to add token here

![66a78a49-44a9-4e93-af36-6f8c93646dd3](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/1ae39f54-02bf-41fa-a949-b684a5b9e981)



<li>Select Secret text</li>
<li>ID - any name</li>

![2f8cbfd0-bc9c-4022-a7ee-040c54c9751b](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/a48be627-a6e2-4209-9a46-f87fd4b25f4b)


Select Token after adding and click on save

![95a01889-fa07-4c40-8149-625dda8e1050](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/0a4c7c92-fdd7-4dee-becb-262ce34ebf2a)


Now go back to Pipeline to verify whether it's working or not, it's absolutely working fine

![Capture](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/ce39dacd-2b42-4dbd-b4e6-2ecb100d019f)

![df](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/0bdf50d5-9d1f-4b64-b956-ed32fa39eb01)


Now going to check SonarQube, it's perfectly working fine

![ldfdj](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/49edf148-770b-45fa-9072-f7c00b479cc9)

Once our code is passed now I am going to deploy it on Docker

<h1>Installing Docker</h1>
Started Instance
<br>
Install Docker
<br>
Update the apt package index and install packages to allow apt to use a repository over HTTPS:

<pre>
    sudo apt-get update
    sudo apt-get install \
    ca-certificates \
    curl \
    gnupg
</pre>

Add Docker’s official GPG key:

<pre>
  sudo install -m 0755 -d /etc/apt/keyrings
  curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
  sudo chmod a+r /etc/apt/keyrings/docker.gpg
</pre>

Use the following command to set up the repository:
<pre>
  echo \
  "deb [arch="$(dpkg --print-architecture)" signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  "$(. /etc/os-release && echo "$VERSION_CODENAME")" stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
</pre>

Update the apt package index:

<pre>sudo apt update
</pre>

To install the latest version, run:

<pre>
  sudo apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
</pre>

<h1>Password-based Server Connectivity</h1>
Now I am going to make connection between Jenkins server and Docker server<br>

After running all the commands go back to the Jenkins server
<pre>
  sudo su jenkins
</pre>

Open Docker server

<pre>
  sudo su 
  nano /etc/ssh/sshd_config
</pre>

Uncomment this first
![8fc77d61-25fc-43b3-92c0-89168303bdff](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/b7645b22-6d7f-4e0b-a35c-d82ff44a2ec0)


And change the Password Authentication to Yes
![ldd](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/0804120a-b01c-4d8b-baa6-9259063d3f6c)


<pre>systemctl restart sshd
</pre>

Back to Jenkins Server
<br>
Now you can see it's asking for the password earlier it's showing permission denied
![;d](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/2a618672-7ff0-485c-ace6-33d3423ab505)


Now we have to change the password of the Docker Ubuntu user
<br>
Back to Docker Server

<pre>passwd ubuntu
</pre>
![Capture](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/a108ff1e-fb6f-4118-821d-7c71d875c857)

Back to Jenkins Server

<pre>ssh ubuntu@172.31.22.109
</pre>

Access done now
![b52a9131-8c1c-4061-acb8-001b98da03a6](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/56a8c30b-be7c-4361-8e00-7656e332ebc9)

Now I am going to generate SSH Key in Jenkins Server


![ddd](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/48319551-8941-49bc-97b3-47177c8c527a)
After generating keygen I entered the command

<pre>ssh-copy-id ubuntu@172.31.22.109</pre>

After running the command enter the password
![jkls](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/5766fb70-c639-4ab6-aa7d-70310d074a96)

<pre>ssh ubuntu@172.31.22.109
</pre>

<h1>Now we don't need to password again anymore</h1>

Back to Jenkins again

<li>Manage Jenkins</li>

<li>Configure system</li>

<li>Server group center</li>

![dlf](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/067ff43f-ff4e-4017-b69b-4a41c17c7219)


Now we have to add a server list
![d](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/756be5db-a34b-4ce1-8296-341acfac8a90)


Now go to Pipeline
<br>
Configure
<br>
Post-build action
<br>
Add build step

![f](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/0afa9aac-4265-42ee-a1fa-c6a1489b68ee)


Now I am gonna build the pipeline to verify whether it's working or not
<br>
And it seems to be working
![Capture](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/1aabef4b-f99e-44bf-9fcb-15184af6d2dc)


Open Docker Server, as we can our file got created here
![c](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/ff325c1a-7239-483c-bdca-7e8ed8c6849f)


Open the Git Repository now

<li>Create a Dockerfile</li>

<li>Commit the file</li>

![inthels](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/e1f8133c-8d63-40df-ab91-b4cbe4376009)

See Auto trigger started

![build](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/f4221d57-b3e5-4056-abdf-e2904db01ac4)


Now returned to Pipeline, configure

<li>I have deleted the Remote shell</li>

<li>Clicked to execute shell</li>

Created a folder in the Docker server



![ins](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/8be29a93-3169-4997-a13d-faa100f3eff4)

Clicked to execute shell and fill the details, here I entered docker server IP and folder which is website


![love](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/0d0e3ec7-b4c0-437f-bbd7-b4b943e8aea4)

We have got the success message here and let us check our docker server

![jldfl](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/07c1e218-f5d9-475d-8440-92fd1fef24ed)


All the files copied to the Docker server
![ddfjslfjld](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/c22e4459-fe7e-475c-8479-4055db4b750f)

<h1>Building the Image and running the container</h1>
Back to the Docker server and need to give permission so that we can run all the commands without sudo


<pre>sudo 
  usermod -aG docker ubuntu 
  newgrp docker
</pre>
After giving the permission we are able to use the docker without sudo

<pre>docker ps
</pre>

![laila](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/a898e9f2-0a59-40ec-b1e2-6c9e4ddf0ba8)

Now back to Jenkins

<li>Click to Pipeline</li>

<li>Click to configure</li>

<li>Click to post-build actions</li>

<li>Select the Remote shell again in the build steps</li>

<li>I gave any random name</li>


![jklkaladfd](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/582b5ace-ba9e-4107-b0bf-843fabb72bed)

Time to check our docker container got created or not
<br>
It got created but we have to add port 8085 in the inbound setting of the security group

![ajjd](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/a65a379f-a5be-4239-b826-aeb55b586151)

Now time to check the public URL of the docker Instance
http://54.227.42.50:8085/ (our code is successfully deployed on docker container)

![jkldldlfdfljdflja;df](https://github.com/samsorrahman/Jenkins-SonarQube-Docker/assets/112087807/f67c2d3a-c822-436f-8d30-bde4f5b04232)


<h1>Don't forget to Follow me for more Projects</h1>
