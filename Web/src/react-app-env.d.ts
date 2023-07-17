/// <reference types="react-scripts" />

declare module '*.less' {
  const content: { [className: string]: string };
  export default content;
}
