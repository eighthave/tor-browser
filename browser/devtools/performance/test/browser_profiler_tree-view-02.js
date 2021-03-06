/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

/**
 * Tests if the profiler's tree view implementation works properly and
 * creates the correct column structure after expanding some of the nodes.
 */

let { CATEGORY_MASK } = devtools.require("devtools/shared/profiler/global");

function test() {
  let { ThreadNode } = devtools.require("devtools/shared/profiler/tree-model");
  let { CallView } = devtools.require("devtools/shared/profiler/tree-view");

  let threadNode = new ThreadNode(gSamples);
  let treeRoot = new CallView({ frame: threadNode });

  let container = document.createElement("vbox");
  treeRoot.autoExpandDepth = 0;
  treeRoot.attachTo(container);

  let $$fun = node => container.querySelectorAll(".call-tree-cell[type=function] > " + node);
  let $$dur = i => container.querySelectorAll(".call-tree-cell[type=duration]")[i];
  let $$perc = i => container.querySelectorAll(".call-tree-cell[type=percentage]")[i];
  let $$sampl = i => container.querySelectorAll(".call-tree-cell[type=samples]")[i];

  is(container.childNodes.length, 1,
    "The container node should have one child available.");
  is(container.childNodes[0].className, "call-tree-item",
    "The root node in the tree has the correct class name.");

  is($$dur(0).getAttribute("value"), "15 ms",
    "The root's duration cell displays the correct value.");
  is($$perc(0).getAttribute("value"), "100%",
    "The root's percentage cell displays the correct value.");
  is($$sampl(0).getAttribute("value"), "4",
    "The root's samples cell displays the correct value.");
  is($$fun(".call-tree-name")[0].getAttribute("value"), "(root)",
    "The root's function cell displays the correct name.");
  is($$fun(".call-tree-url")[0].getAttribute("value"), "",
    "The root's function cell displays the correct url.");
  is($$fun(".call-tree-line")[0].getAttribute("value"), "",
    "The root's function cell displays the correct line.");
  is($$fun(".call-tree-host")[0].getAttribute("value"), "",
    "The root's function cell displays the correct host.");
  is($$fun(".call-tree-category")[0].getAttribute("value"), "",
    "The root's function cell displays the correct category.");

  treeRoot.expand();

  is(container.childNodes.length, 2,
    "The container node should have two children available.");
  is(container.childNodes[0].className, "call-tree-item",
    "The root node in the tree has the correct class name.");
  is(container.childNodes[1].className, "call-tree-item",
    "The .A node in the tree has the correct class name.");

  is($$dur(1).getAttribute("value"), "15 ms",
    "The .A node's duration cell displays the correct value.");
  is($$perc(1).getAttribute("value"), "100%",
    "The .A node's percentage cell displays the correct value.");
  is($$sampl(1).getAttribute("value"), "4",
    "The .A node's samples cell displays the correct value.");
  is($$fun(".call-tree-name")[1].getAttribute("value"), "A",
    "The .A node's function cell displays the correct name.");
  is($$fun(".call-tree-url")[1].getAttribute("value"), "baz",
    "The .A node's function cell displays the correct url.");
  ok($$fun(".call-tree-url")[1].getAttribute("tooltiptext").contains("http://foo/bar/baz"),
    "The .A node's function cell displays the correct url tooltiptext.");
  is($$fun(".call-tree-line")[1].getAttribute("value"), ":12",
    "The .A node's function cell displays the correct line.");
  is($$fun(".call-tree-host")[1].getAttribute("value"), "foo",
    "The .A node's function cell displays the correct host.");
  is($$fun(".call-tree-category")[1].getAttribute("value"), "Gecko",
    "The .A node's function cell displays the correct category.");

  let A = treeRoot.getChild();
  A.expand();

  is(container.childNodes.length, 4,
    "The container node should have four children available.");
  is(container.childNodes[2].className, "call-tree-item",
    "The .B node in the tree has the correct class name.");
  is(container.childNodes[3].className, "call-tree-item",
    "The .E node in the tree has the correct class name.");

  is($$dur(2).getAttribute("value"), "8 ms",
    "The .A.B node's duration cell displays the correct value.");
  is($$perc(2).getAttribute("value"), "75%",
    "The .A.B node's percentage cell displays the correct value.");
  is($$sampl(2).getAttribute("value"), "3",
    "The .A.B node's samples cell displays the correct value.");
  is($$fun(".call-tree-name")[2].getAttribute("value"), "B",
    "The .A.B node's function cell displays the correct name.");
  is($$fun(".call-tree-url")[2].getAttribute("value"), "baz",
    "The .A.B node's function cell displays the correct url.");
  ok($$fun(".call-tree-url")[2].getAttribute("tooltiptext").contains("http://foo/bar/baz"),
    "The .A.B node's function cell displays the correct url tooltiptext.");
  is($$fun(".call-tree-line")[2].getAttribute("value"), ":34",
    "The .A.B node's function cell displays the correct line.");
  is($$fun(".call-tree-host")[2].getAttribute("value"), "foo",
    "The .A.B node's function cell displays the correct host.");
  is($$fun(".call-tree-category")[2].getAttribute("value"), "Styles",
    "The .A.B node's function cell displays the correct category.");

  is($$dur(3).getAttribute("value"), "7 ms",
    "The .A.E node's duration cell displays the correct value.");
  is($$perc(3).getAttribute("value"), "25%",
    "The .A.E node's percentage cell displays the correct value.");
  is($$sampl(3).getAttribute("value"), "1",
    "The .A.E node's samples cell displays the correct value.");
  is($$fun(".call-tree-name")[3].getAttribute("value"), "E",
    "The .A.E node's function cell displays the correct name.");
  is($$fun(".call-tree-url")[3].getAttribute("value"), "baz",
    "The .A.E node's function cell displays the correct url.");
  ok($$fun(".call-tree-url")[3].getAttribute("tooltiptext").contains("http://foo/bar/baz"),
    "The .A.E node's function cell displays the correct url tooltiptext.");
  is($$fun(".call-tree-line")[3].getAttribute("value"), ":90",
    "The .A.E node's function cell displays the correct line.");
  is($$fun(".call-tree-host")[3].getAttribute("value"), "foo",
    "The .A.E node's function cell displays the correct host.");
  is($$fun(".call-tree-category")[3].getAttribute("value"), "GC",
    "The .A.E node's function cell displays the correct category.");

  finish();
}

let gSamples = [{
  time: 5,
  frames: [
    { category: CATEGORY_MASK('other'),  location: "(root)" },
    { category: CATEGORY_MASK('other'),  location: "A (http://foo/bar/baz:12)" },
    { category: CATEGORY_MASK('css'),    location: "B (http://foo/bar/baz:34)" },
    { category: CATEGORY_MASK('js'),     location: "C (http://foo/bar/baz:56)" }
  ]
}, {
  time: 5 + 1,
  frames: [
    { category: CATEGORY_MASK('other'),  location: "(root)" },
    { category: CATEGORY_MASK('other'),  location: "A (http://foo/bar/baz:12)" },
    { category: CATEGORY_MASK('css'),    location: "B (http://foo/bar/baz:34)" },
    { category: CATEGORY_MASK('gc', 1),  location: "D (http://foo/bar/baz:78)" }
  ]
}, {
  time: 5 + 1 + 2,
  frames: [
    { category: CATEGORY_MASK('other'),  location: "(root)" },
    { category: CATEGORY_MASK('other'),  location: "A (http://foo/bar/baz:12)" },
    { category: CATEGORY_MASK('css'),    location: "B (http://foo/bar/baz:34)" },
    { category: CATEGORY_MASK('gc', 1),  location: "D (http://foo/bar/baz:78)" }
  ]
}, {
  time: 5 + 1 + 2 + 7,
  frames: [
    { category: CATEGORY_MASK('other'),   location: "(root)" },
    { category: CATEGORY_MASK('other'),   location: "A (http://foo/bar/baz:12)" },
    { category: CATEGORY_MASK('gc', 2),   location: "E (http://foo/bar/baz:90)" },
    { category: CATEGORY_MASK('network'), location: "F (http://foo/bar/baz:99)" }
  ]
}];
