/* Reusable quiz widget for teaching lessons.
 * HTML contract:
 *   <div class="quiz" data-quiz>
 *     <div class="qblock">
 *       <p class="q">Question text</p>
 *       <div class="diagram">optional ascii</div>
 *       <button class="opt" data-correct>Option A</button>
 *       <button class="opt">Option B</button>
 *       ...
 *       <p class="explain">Explanation shown after answering.</p>
 *     </div>
 *     ... more qblocks ...
 *   </div>
 * Exactly one .opt per qblock must carry data-correct.
 */
document.querySelectorAll('[data-quiz]').forEach(function (quiz) {
  quiz.querySelectorAll('.qblock').forEach(function (block) {
    var options = block.querySelectorAll('.opt');
    options.forEach(function (opt) {
      opt.addEventListener('click', function () {
        if (block.classList.contains('solved')) return;
        block.classList.add('solved');
        options.forEach(function (o) { o.disabled = true; });
        if (opt.hasAttribute('data-correct')) {
          opt.classList.add('correct');
        } else {
          opt.classList.add('wrong');
          block.querySelector('.opt[data-correct]').classList.add('correct');
        }
      });
    });
  });
});
