---


---

<h1 id="microakka--akka--framework-in-c--on-microcontrollers">microAkka : Akka  framework in C++  on microcontrollers</h1>
<p>C++ Actor Framework for Embedded Systems - <a href="https://akka.io/">Akka</a> alike</p>
<p><img src="doc/ESP8266.jpg" height="100"><img src="doc/ESP32.png" height="100"></p>
<h2 id="target">Target</h2>
<p>The purpose is to provide a  C++ framework for writing message driven actors.</p>
<p>Seen the popularity of the Lightbend Akka framework and its extensive documentation and features, I decided to build this<br>
framework on the same principles and naming conventions. Saves me some time on documenting my own stuff. ;-)</p>
<p>The article :<br>
<a href="https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6">https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6</a><br>
was very helpful to understand the inner working of Akka framework.</p>
<p>My intention is that a central brain running on PC or server can interact with IOT devices, the communication between both should be transparently<br>
as communicating local between actors. The actor systems can bridge communication between devices  using MQTT.</p>
<p>The MQTT topic and message are based on some conventions to ease integration.</p>
<h4 id="topicsmessages-for-request-reply-pattern">Topics&amp;Messages for request-reply pattern</h4>
<ul>
<li>each actor has several  paths or addresses on MQTT :  for listening for messages destined to this device-actor : <em><strong>dst/ActorSystem/Actor/MessageClass</strong></em></li>
<li>The MessageClass is extended with “Reply” to specify the message type reply</li>
<li>message is a JSON object with a reply address in “<span class="katex--inline"><span class="katex"><span class="katex-mathml"><math><semantics><mrow><mi>s</mi><mi>r</mi><mi>c</mi><mi mathvariant="normal">&amp;quot;</mi><mi>f</mi><mi>i</mi><mi>e</mi><mi>l</mi><mi>d</mi><mi>a</mi><mi>n</mi><mi>d</mi><mi mathvariant="normal">&amp;quot;</mi></mrow><annotation encoding="application/x-tex">src&amp;quot; field and &amp;quot;</annotation></semantics></math></span><span class="katex-html" aria-hidden="true"><span class="base"><span class="strut" style="height: 0.88888em; vertical-align: -0.19444em;"></span><span class="mord mathit">s</span><span class="mord mathit" style="margin-right: 0.02778em;">r</span><span class="mord mathit">c</span><span class="mord">"</span><span class="mord mathit" style="margin-right: 0.10764em;">f</span><span class="mord mathit">i</span><span class="mord mathit">e</span><span class="mord mathit" style="margin-right: 0.01968em;">l</span><span class="mord mathit">d</span><span class="mord mathit">a</span><span class="mord mathit">n</span><span class="mord mathit">d</span><span class="mord">"</span></span></span></span></span>id” for each request to be used in the reply. The rest of the field names can be freely chosen specific for the Request/Reply handled.</li>
</ul>
<h4 id="topicsmessages-for-event-pattern">Topics&amp;Messages for event pattern</h4>
<ul>
<li>some for emitting events and properties : <em><strong>src/ActorSystem/Actor/Property</strong></em></li>
<li>The values are most of the time simple singular JSON values</li>
</ul>
<h4 id="examples">Examples</h4>
<ul>
<li>Event system upTime : src/ESP32-7847/system/upTime : 4643244</li>
<li>Request get Actor ‘system’ properties : dst/ESP32-63147/system/properties : {“src”:“master/brain”,“id”:1234 }</li>
<li>Reply to Actor master/brain : dst/master/brain/propertiesReply :{“id”:122,“ssid”:“MySsid”,“ip”:“192.168.0.157”,“mac”:“AE:38:FB:3F:0A:00”,“src”:“ESP32-63147/wifi”}</li>
</ul>
<h2 id="design-decisions">Design decisions</h2>
<ul>
<li>The interface is close to the Java API of Akka as C++ translation was easier compared to the Scala interface.</li>
<li>To reduce resource consumptions in a limited embedded environment some design aspects are different from Akka Java/Scala.</li>
<li>actors can share the same mailbox, each mailbox has 1 thread ( MessageDispatcher ) to invoke the actors. On FreeRtos based controllers multiple threads ( aka Tasks ) can be started running dispatchers with multiple mailboxes. On Arduino common platform this will be likely limited to 1 thread. This is close to a coroutine model.</li>
<li>C++ has limited introspection facilities, so message classes are put explicitly into the message</li>
<li>the message passed between actors is in a <strong>in-memory serialized form</strong>, based on aspects from <strong>Xdr</strong> ( 4 byte granularity ) and <strong>Protobuf</strong> ( each element is tagged ). The message is copied and a pointer to this copy is passed on. Since most controllers are already 32 bit word aligned, the Xdr form should speed up data retrieval.</li>
<li>little attention has been given on <strong>stopping actors</strong> as in an embedded environment these actors are started once and run forever, so no resource cleanup yet there.</li>
<li>Unique id’s are created based on FNV hashing, when compiler optimization is activated this is executed at compile time and not run-time. These <strong>unique id’s</strong> are used for string references in 16 bit and actor references. These 16 bit hashes speed up comparison and extraction</li>
<li>A number of building blocks supporting an Akka framework are provided by an RTOS like <strong>FreeRTOS</strong>. So as I first started without an RTOS the result was simulating a lot of an RTOS was offering out of the box. So seen the support of Amazon for FreeRTOS and the general availability for different processors it was an easy arbitration to go for this approach. See also : <a href="https://community.arm.com/iot/embedded/b/embedded-blog/posts/beyond-the-rtos-a-better-way-to-design-real-time-embedded-software">https://community.arm.com/iot/embedded/b/embedded-blog/posts/beyond-the-rtos-a-better-way-to-design-real-time-embedded-software</a> . It was even possible to keep the approach of fine tuning the code first on a linux platform and deploy then on a microcontroller as there exists FreeRTOS simulators on Linux.</li>
</ul>
<h2 id="platforms-supported">Platforms supported</h2>
<ul>
<li>Linux ( Debian ), should work on all linux versions - repository microAkka</li>
<li>ESP32 with ESP-IDF - repository <a href="https://github.com/vortex314/akkaEsp32">akkaEsp32</a></li>
<li>ESP8266 with ESP-OPEN-RTOS - repository <a href="https://github.com/vortex314/akkaEsp8266">akkaEsp8266</a></li>
<li>Arduino : see <a href="https://github.com/vortex314/akkaArduino">akkaArduino</a>  (  ESP8266 )</li>
</ul>
<h3 id="prerequisites">Prerequisites</h3>
<ul>
<li>C++ 11 compiler</li>
<li>MQTT library dependent on platform</li>
<li>Common repository with platform specifics</li>
</ul>
<h3 id="example">Example</h3>
<p>An actor that replies with an increment of a counter</p>
<p>C++ code</p>
<pre><code>__________________________________________________ main

int main() {

Sys::init();
Mailbox defaultMailbox("default", 100); // nbr of messages in queue max
MessageDispatcher defaultDispatcher;
ActorSystem actorSystem(Sys::hostname(), defaultDispatcher, defaultMailbox);

ActorRef sender = actorSystem.actorOf&lt;Sender&gt;("sender"); // will also start echo actor
ActorRef system = actorSystem.actorOf&lt;System&gt;("System");
ActorRef nnPid = actorSystem.actorOf&lt;NeuralPid&gt;("neuralPid");
ActorRef mqtt = actorSystem.actorOf&lt;Mqtt&gt;("mqtt", "tcp://limero.ddns.net:1883");
ActorRef bridge = actorSystem.actorOf&lt;Bridge&gt;("bridge",mqtt);

defaultDispatcher.attach(defaultMailbox);
defaultDispatcher.unhandled(bridge.cell());
defaultDispatcher.execute();
__________________________________________________ Echo.cpp

Echo::Echo(va_list args)  {}
Echo::~Echo() {}

Receive&amp; Echo::createReceive() {
	return receiveBuilder()
			.match(PING,
	[this](Envelope&amp; msg) {
		uint32_t counter;
		assert(msg.get("counter", counter)==0);
		sender().tell(Msg(PONG)("counter",counter+1),self());
	})
	.build();
}
</code></pre>
<p>Java / Scala code</p>
<pre><code>Coming
</code></pre>
<h3 id="installing">Installing</h3>
<ul>
<li>
<p>Download git repo <a href="https://github.com/vortex314/microAkka">https://github.com/vortex314/microAkka</a></p>
</li>
<li>
<p>Download git repo <a href="https://github.com/vortex314/Common">https://github.com/vortex314/Common</a></p>
</li>
<li>
<p>Download git repo <a href="https://github.com/bblanchon/ArduinoJson">https://github.com/bblanchon/ArduinoJson</a></p>
<ul>
<li>set env variables and mqtt URL</li>
</ul>
<pre><code>Give the example
</code></pre>
<p>End with an example of getting some data out of the system or using it for a little demo</p>
</li>
</ul>
<h2 id="running-the-tests">Running the tests</h2>
<p>Explain how to run the automated tests for this system</p>
<h3 id="break-down-into-end-to-end-tests">Break down into end to end tests</h3>
<p>Explain what these tests test and why</p>
<pre><code>Give an example
</code></pre>
<h3 id="and-coding-style-tests">And coding style tests</h3>
<p>Explain what these tests test and why</p>
<pre><code>Give an example
</code></pre>
<h2 id="deployment">Deployment</h2>
<p>Add additional notes about how to deploy this on a live system</p>
<h2 id="built-with">Built With</h2>
<ul>
<li><a href="http://www.dropwizard.io/1.0.2/docs/">Codelite</a> - The  C++ IDE  used</li>
<li><a href="https://doc.akka.io/docs/akka/2.5/general/actor-systems.html">Akka doc</a> - Documentation</li>
<li><a href="https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6">How Akka Actors work </a> - Used to generate RSS Feeds</li>
</ul>
<h2 id="versioning">Versioning</h2>
<p>We use <a href="http://semver.org/">SemVer</a> for versioning. For the versions available, see the <a href="https://github.com/vortex314/microAkka/tags">tags on this repository</a>.</p>
<h2 id="authors">Authors</h2>
<ul>
<li><strong>Lieven Merckx</strong> - <em>Initial work</em> -</li>
</ul>
<h2 id="license">License</h2>
<p>This project is licensed under the MIT License - see the <a href="LICENSE.md">LICENSE.md</a> file for details</p>

